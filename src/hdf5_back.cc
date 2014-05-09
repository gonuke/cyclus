// hdf5_back.cc
#include "hdf5_back.h"

#include <cmath>
#include <string.h>

#include "blob.h"

namespace cyclus {

Hdf5Back::Hdf5Back(std::string path) : path_(path) {
  hasher_ = Sha1();
  if (boost::filesystem::exists(path_))
    file_ = H5Fopen(path_.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
  else  
    file_ = H5Fcreate(path_.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  opened_types_ = std::set<hid_t>();
  vldatasets_ = std::map<std::string, hid_t>();
  vldts_ = std::map<DbTypes, hid_t>();
  vlkeys_ = std::map<DbTypes, std::set<Digest> >();

  uuid_type_ = H5Tcopy(H5T_C_S1);
  H5Tset_size(uuid_type_, CYCLUS_UUID_SIZE);
  H5Tset_strpad(uuid_type_, H5T_STR_NULLPAD);
  opened_types_.insert(uuid_type_);

  hsize_t sha1_len = CYCLUS_SHA1_NINT;  // 160 bits == 32 bits / int  * 5 ints
  sha1_type_ = H5Tarray_create2(H5T_NATIVE_UINT, 1, &sha1_len);
  opened_types_.insert(sha1_type_);

  vlstr_type_ = H5Tcopy(H5T_C_S1);
  H5Tset_size(vlstr_type_, H5T_VARIABLE);
  opened_types_.insert(vlstr_type_);
  vldts_[VL_STRING] = vlstr_type_;

  blob_type_ = vlstr_type_;
  vldts_[BLOB] = blob_type_;
}

Hdf5Back::~Hdf5Back() {
  // cleanup HDF5
  Hdf5Back::Flush();
  H5Fclose(file_);
  std::set<hid_t>::iterator t;
  for (t = opened_types_.begin(); t != opened_types_.end(); ++t) 
    H5Tclose(*t);
  std::map<std::string, hid_t>::iterator vldsit;
  for (vldsit = vldatasets_.begin(); vldsit != vldatasets_.end(); ++vldsit)
    H5Dclose(vldsit->second);

  // cleanup memory
  std::map<std::string, size_t*>::iterator it;
  std::map<std::string, DbTypes*>::iterator dbtit;
  for (it = tbl_offset_.begin(); it != tbl_offset_.end(); ++it) {
    delete[](it->second);
  }
  for (it = tbl_sizes_.begin(); it != tbl_sizes_.end(); ++it) {
    delete[](it->second);
  }
  for (dbtit = tbl_types_.begin(); dbtit != tbl_types_.end(); ++dbtit) {
    delete[](dbtit->second);
  }
};

void Hdf5Back::Notify(DatumList data) {
  std::map<std::string, DatumList> groups;
  for (DatumList::iterator it = data.begin(); it != data.end(); ++it) {
    std::string name = (*it)->title();
    if (tbl_size_.count(name) == 0) {
      Datum* d = *it;
      CreateTable(d);
    }
    groups[name].push_back(*it);
  }

  std::map<std::string, DatumList>::iterator it;
  for (it = groups.begin(); it != groups.end(); ++it) {
    WriteGroup(it->second);
  }
}

QueryResult Hdf5Back::Query(std::string table, std::vector<Cond>* conds) {
  int i;
  int j;
  herr_t status = 0;
  hid_t tb_set = H5Dopen2(file_, table.c_str(), H5P_DEFAULT);
  hid_t tb_space = H5Dget_space(tb_set);
  hid_t tb_plist = H5Dget_create_plist(tb_set);
  hid_t tb_type = H5Dget_type(tb_set);
  size_t tb_typesize = H5Tget_size(tb_type);
  int tb_length = H5Sget_simple_extent_npoints(tb_space);
  hsize_t tb_chunksize;
  H5Pget_chunk(tb_plist, 1, &tb_chunksize);
  unsigned int nchunks = (tb_length/tb_chunksize) + (tb_length%tb_chunksize == 0?0:1);
  unsigned int n = 0;

  // set up field-conditions map
  std::map<std::string, std::vector<Cond*> > field_conds = std::map<std::string, 
                                                             std::vector<Cond*> >();
  if (conds != NULL) {
    Cond* cond;
    for (i = 0; i < conds->size(); ++i) {
      cond = &((*conds)[i]);
      if (field_conds.count(cond->field) == 0)
        field_conds[cond->field] = std::vector<Cond*>();
      field_conds[cond->field].push_back(cond);
    }
  }

  // read in data
  QueryResult qr = GetTableInfo(table, tb_set, tb_type);
  int nfields = qr.fields.size();
  for (i = 0; i < nfields; ++i)
    if (field_conds.count(qr.fields[i]) == 0)
      field_conds[qr.fields[i]] = std::vector<Cond*>();
  for (n; n < nchunks; ++n) {
    hsize_t start = n * tb_chunksize;
    hsize_t count = (tb_length-start)<tb_chunksize ? tb_length - start : tb_chunksize;
    char* buf = new char [tb_typesize * count];
    hid_t memspace = H5Screate_simple(1, &count, NULL);
    status = H5Sselect_hyperslab(tb_space, H5S_SELECT_SET, &start, NULL, &count, NULL);
    status = H5Dread(tb_set, tb_type, memspace, tb_space, H5P_DEFAULT, buf);
    int offset = 0;
    bool is_valid_row;
    QueryRow row = QueryRow(nfields);
    for (i = 0; i < count; ++i) {
      offset = i * tb_typesize;
      is_valid_row = true;
      for (j = 0; j < nfields; ++j) {
        switch (qr.types[j]) {
          case BOOL: {
            throw IOError("booleans not yet implemented for HDF5.");
            break;
          }
          case INT: {
            int x = *reinterpret_cast<int*>(buf + offset);
            is_valid_row = CmpConds<int>(&x, &(field_conds[qr.fields[j]]));
            if (is_valid_row)
              row[j] = x;
            break;
          }
          case FLOAT: {
            float x = *reinterpret_cast<float*>(buf + offset);
            is_valid_row = CmpConds<float>(&x, &(field_conds[qr.fields[j]]));
            if (is_valid_row)
              row[j] = x;
            break;
          }
          case DOUBLE: {
            double x = *reinterpret_cast<double*>(buf + offset);
            is_valid_row = CmpConds<double>(&x, &(field_conds[qr.fields[j]]));
            if (is_valid_row)
              row[j] = x;
            break;
          }
          case STRING: {
            std::string x = std::string(buf + offset, tbl_sizes_[table][j]);
            size_t nullpos = x.find('\0');
            if (nullpos >= 0)
              x.resize(nullpos);
            is_valid_row = CmpConds<std::string>(&x, &(field_conds[qr.fields[j]]));
            if (is_valid_row)
              row[j] = x;
            break;
          }
          case VL_STRING: {
            std::string x = VLRead<std::string, VL_STRING>(buf + offset);
            is_valid_row = CmpConds<std::string>(&x, &(field_conds[qr.fields[j]]));
            if (is_valid_row)
              row[j] = x;
            break;
          }
          case BLOB: {
            Blob x (std::string(buf + offset, sizeof(char *)));
            is_valid_row = CmpConds<Blob>(&x, &(field_conds[qr.fields[j]]));
            if (is_valid_row)
              row[j] = x;
            break;
          }
          case UUID: {
            boost::uuids::uuid x;
            memcpy(&x, buf + offset, 16);
            is_valid_row = CmpConds<boost::uuids::uuid>(&x, &(field_conds[qr.fields[j]]));
            if (is_valid_row)
              row[j] = x;
            break;
          }
        }
        if (!is_valid_row)
          break;
        offset += tbl_sizes_[table][j];
      }
      if (is_valid_row) {
        qr.rows.push_back(row);
        QueryRow row = QueryRow(nfields);
      } else {
        row.clear();
        row.reserve(nfields);
      }
    }
    delete[] buf;
    H5Sclose(memspace);
  }

  // close and return
  H5Tclose(tb_type);
  H5Pclose(tb_plist);
  H5Sclose(tb_space);
  H5Dclose(tb_set);
  return qr;
}

QueryResult Hdf5Back::GetTableInfo(std::string title, hid_t dset, hid_t dt) {
  int i;
  char * colname;
  hsize_t ncols = H5Tget_nmembers(dt);
  std::string fieldname;
  std::string fieldtype;
  LoadTableTypes(title, dset, ncols);
  DbTypes* dbtypes = tbl_types_[title];

  QueryResult qr;
  for (i = 0; i < ncols; ++i) {
    colname = H5Tget_member_name(dt, i);
    fieldname = std::string(colname);
    free(colname);
    qr.fields.push_back(fieldname);
    qr.types.push_back(dbtypes[i]);
  }
  return qr;
}

void Hdf5Back::LoadTableTypes(std::string title, hsize_t ncols) {
  if (tbl_types_.count(title) > 0)
    return;
  hid_t dset = H5Dopen2(file_, title.c_str(), H5P_DEFAULT);
  LoadTableTypes(title, dset, ncols);
  H5Dclose(dset);
}

void Hdf5Back::LoadTableTypes(std::string title, hid_t dset, hsize_t ncols) {
  if (tbl_types_.count(title) > 0)
    return;

  // get types from db
  int dbt[ncols];
  hid_t dbtypes_attr = H5Aopen(dset, "cyclus_dbtypes", H5P_DEFAULT);
  hid_t dbtypes_type = H5Aget_type(dbtypes_attr);
  H5Aread(dbtypes_attr, dbtypes_type, dbt);
  H5Tclose(dbtypes_type);
  H5Aclose(dbtypes_attr);

  // store types on class
  DbTypes* dbtypes = new DbTypes[ncols];
  for (int i = 0; i < ncols; ++i)
    dbtypes[i] = static_cast<DbTypes>(dbt[i]);
  tbl_types_[title] = dbtypes;
}

std::string Hdf5Back::Name() {
  return path_;
}

void Hdf5Back::CreateTable(Datum* d) {
  Datum::Vals vals = d->vals();
  hsize_t nvals = vals.size();
  Datum::Shape shape;
  Datum::Shapes shapes = d->shapes();

  size_t dst_size = 0;
  size_t* dst_offset = new size_t[nvals];
  size_t* dst_sizes = new size_t[nvals];
  hid_t field_types[nvals];
  DbTypes* dbtypes = new DbTypes[nvals];
  const char* field_names[nvals];
  for (int i = 0; i < nvals; ++i) {
    dst_offset[i] = dst_size;
    field_names[i] = vals[i].first;
    const std::type_info& valtype = vals[i].second.type();
    if (valtype == typeid(bool)) {
      dbtypes[i] = BOOL;
      field_types[i] = H5T_NATIVE_CHAR;
      dst_sizes[i] = sizeof(char);
      dst_size += sizeof(char);
    }
    else if (valtype == typeid(int)) {
      dbtypes[i] = INT;
      field_types[i] = H5T_NATIVE_INT;
      dst_sizes[i] = sizeof(int);
      dst_size += sizeof(int);
    } else if (valtype == typeid(float)) {
      dbtypes[i] = FLOAT;
      field_types[i] = H5T_NATIVE_FLOAT;
      dst_sizes[i] = sizeof(float);
      dst_size += sizeof(float);
    } else if (valtype == typeid(double)) {
      dbtypes[i] = DOUBLE;
      field_types[i] = H5T_NATIVE_DOUBLE;
      dst_sizes[i] = sizeof(double);
      dst_size += sizeof(double);
    } else if (valtype == typeid(std::string)) {
      shape = shapes[i];
      if (shape == NULL || (*shape)[0] < 1) {
        dbtypes[i] = VL_STRING;
        field_types[i] = sha1_type_;
        dst_sizes[i] = CYCLUS_SHA1_SIZE;
        dst_size += CYCLUS_SHA1_SIZE;
      } else {
        dbtypes[i] = STRING;
        field_types[i] = H5Tcopy(H5T_C_S1);
        H5Tset_size(field_types[i], (*shape)[0]);
        H5Tset_strpad(field_types[i], H5T_STR_NULLPAD);
        opened_types_.insert(field_types[i]);
        dst_sizes[i] = sizeof(char) * (*shape)[0];
        dst_size += sizeof(char) * (*shape)[0];
      }
    } else if (valtype == typeid(Blob)) {
      dbtypes[i] = BLOB;
      field_types[i] = blob_type_;
      dst_sizes[i] = CYCLUS_SHA1_SIZE;
      dst_size += CYCLUS_SHA1_SIZE;
    } else if (valtype == typeid(boost::uuids::uuid)) {
      dbtypes[i] = UUID;
      field_types[i] = uuid_type_;
      dst_sizes[i] = CYCLUS_UUID_SIZE;
      dst_size += CYCLUS_UUID_SIZE;
    } 
  }

  herr_t status;
  const char* title = d->title().c_str();
  int compress = 1;
  int chunk_size = 1000;
  void* fill_data = NULL;
  void* data = NULL;

  // Make the table
  status = H5TBmake_table(title, file_, title, nvals, 0, dst_size,
                          field_names, dst_offset, field_types, chunk_size, 
                          fill_data, compress, data);

  // add dbtypes attribute
  hid_t tb_set = H5Dopen2(file_, title, H5P_DEFAULT);
  hid_t attr_space = H5Screate(H5S_SCALAR);
  hid_t dbtypes_type = H5Tarray_create2(H5T_NATIVE_INT, 1, &nvals);
  hid_t dbtypes_attr = H5Acreate2(tb_set, "cyclus_dbtypes", dbtypes_type, attr_space, H5P_DEFAULT, H5P_DEFAULT);
  H5Awrite(dbtypes_attr, dbtypes_type, dbtypes);
  H5Aclose(dbtypes_attr);
  H5Tclose(dbtypes_type);
  H5Sclose(attr_space);
  H5Dclose(tb_set);

  // record everything for later
  tbl_offset_[d->title()] = dst_offset;
  tbl_size_[d->title()] = dst_size;
  tbl_sizes_[d->title()] = dst_sizes;
  tbl_types_[d->title()] = dbtypes;
}

void Hdf5Back::WriteGroup(DatumList& group) {
  std::string title = group.front()->title();

  size_t* offsets = tbl_offset_[title];
  size_t* sizes = tbl_sizes_[title];
  size_t rowsize = tbl_size_[title];

  char* buf = new char[group.size() * rowsize];
  FillBuf(title, buf, group, sizes, rowsize);

  herr_t status = H5TBappend_records(file_, title.c_str(), group.size(), rowsize,
                              offsets, sizes, buf);
  if (status < 0) {
    throw IOError("Failed to write some data to hdf5 output db");
  }
  delete[] buf;
}

void Hdf5Back::FillBuf(std::string title, char* buf, DatumList& group, 
                       size_t* sizes, size_t rowsize) {
  Datum::Vals vals;
  Datum::Vals header = group.front()->vals();
  int ncols = header.size();
  LoadTableTypes(title, ncols);
  DbTypes* dbtypes = tbl_types_[title];

  size_t offset = 0;
  const void* val;
  DatumList::iterator it;
  for (it = group.begin(); it != group.end(); ++it) {
    for (int col = 0; col < ncols; ++col) {
      vals = (*it)->vals();
      const boost::spirit::hold_any* a = &(vals[col].second);
      switch (dbtypes[col]) {
        case BOOL: {
          throw IOError("booleans not yet implemented for HDF5.");
          break;
        }
        case INT:
        case FLOAT:
        case DOUBLE: {
          val = a->castsmallvoid();
          memcpy(buf + offset, val, sizes[col]);
          break;
        }
        case STRING: {
          const std::string s = a->cast<std::string>();
          size_t fieldlen = sizes[col];
          size_t slen = std::min(s.size(), fieldlen);
          memcpy(buf + offset, s.c_str(), slen);
          memset(buf + offset + slen, 0, fieldlen - slen);
          break;
        }
        case VL_STRING: {
          Digest key = VLWrite<std::string, VL_STRING>(a);
          memcpy(buf + offset, key.val, CYCLUS_SHA1_SIZE);
          break;
        }
        case BLOB: {
          // TODO: fix this memory leak, but the copied bytes must remain
          // valid until the hdf5 file is flushed or closed.
          std::string s = a->cast<Blob>().str();
          char* v = new char[strlen(s.c_str())];
          strcpy(v, s.c_str());
          memcpy(buf + offset, &v, sizes[col]);
          break;
        }
        case UUID: {
          boost::uuids::uuid uuid = a->cast<boost::uuids::uuid>();
          memcpy(buf + offset, &uuid, CYCLUS_UUID_SIZE);
          break;
        }
      }
      offset += sizes[col];
    }
  }
}

template <typename T, DbTypes U>
T Hdf5Back::VLRead(const char* rawkey) { 
  // key is used as offset
  Digest key;
  memcpy(key.val, rawkey, CYCLUS_SHA1_SIZE);
  const std::vector<hsize_t> idx = key.cast<hsize_t>();
  hid_t dset = VLDataset(U, false);
  hid_t dspace = H5Dget_space(dset);
  hsize_t extent[CYCLUS_SHA1_NINT] = {1, 1, 1, 1, 1};
  hid_t mspace = H5Screate_simple(CYCLUS_SHA1_NINT, extent, NULL);
  herr_t status = H5Sselect_hyperslab(dspace, H5S_SELECT_SET, (const hsize_t*) &idx[0], 
                                      NULL, extent, NULL);
  if (status < 0)
    throw IOError("could not select hyperslab of value array for reading.");
  char** buf = new char* [sizeof(char *)];
  status = H5Dread(dset, vldts_[U], mspace, dspace, H5P_DEFAULT, buf);
  if (status < 0)
    throw IOError("failed to read in variable length data.");
  T val = T(buf[0]);
  status = H5Dvlen_reclaim(vldts_[U], mspace, H5P_DEFAULT, buf);
  if (status < 0)
    throw IOError("failed to reclaim variable lenght data space.");
  delete[] buf;
  H5Sclose(mspace);
  H5Sclose(dspace);
  return val;
}

template <typename T, DbTypes U>
Digest Hdf5Back::VLWrite(T x) {
  hasher_.Clear();
  hasher_.Update(x);
  Digest key = hasher_.digest();
  hid_t keysds = VLDataset(U, true);
  hid_t valsds = VLDataset(U, false);
  if (vlkeys_[U].count(key) == 1)
    return key;
  AppendVLKey(keysds, U, key);
  InsertVLVal(valsds, U, key, x);
  return key;
}

hid_t Hdf5Back::VLDataset(DbTypes dbtype, bool forkeys) {
  std::string name;
  switch (dbtype) {
    case VL_STRING: {
      name = "String";
      break;
    }
    case BLOB: {
      name = "Blob";
      break;
    }
  }
  name += forkeys ? "Keys" : "Vals";

  // already opened
  if (vldatasets_.count(name) > 0)
    return vldatasets_[name];

  // already in db
  hid_t dset;
  hid_t dspace;
  herr_t status;
  if (H5Lexists(file_, name.c_str(), H5P_DEFAULT)) {
    dset = H5Dopen2(file_, name.c_str(), H5P_DEFAULT);
    if (forkeys) {
      // read in existing keys to vlkeys_
      dspace = H5Dget_space(dset);
      unsigned int nkeys = H5Sget_simple_extent_npoints(dspace);
      char* buf = new char [CYCLUS_SHA1_SIZE * nkeys];
      status = H5Dread(dset, sha1_type_, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);
      if (status < 0)
        throw IOError("failed to read in keys for " + name);
      for (int n = 0; n < nkeys; ++n) {
        Digest d = Digest();
        memcpy(buf + (n * CYCLUS_SHA1_SIZE), &d.val, CYCLUS_SHA1_SIZE);
        vlkeys_[dbtype].insert(d);
      }
      H5Sclose(dspace);
      delete[] buf;
    }
    vldatasets_[name] = dset;
    return dset;
  }

  // doesn't exist at all
  hid_t dt;
  hid_t prop;
  if (forkeys) {
    hsize_t dims[1] = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hsize_t chunkdims[1] = {512};  // this is a 10 kb chunksize
    dt = sha1_type_;
    dspace = H5Screate_simple(1, dims, maxdims);
    prop = H5Pcreate(H5P_DATASET_CREATE);
    status = H5Pset_chunk(prop, 1, chunkdims);
    if (status < 0) 
      throw IOError("could not create HDF5 array " + name);
  } else {
    hsize_t dims[CYCLUS_SHA1_NINT] = {UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX};
    hsize_t chunkdims[CYCLUS_SHA1_NINT] = {1, 1, 1, 1, 1};  // this is a single element
    dt = vldts_[dbtype];
    dspace = H5Screate_simple(CYCLUS_SHA1_NINT, dims, dims);
    prop = H5Pcreate(H5P_DATASET_CREATE);
    status = H5Pset_chunk(prop, CYCLUS_SHA1_NINT, chunkdims);
    if (status < 0) 
      throw IOError("could not create HDF5 array " + name);
  }
  dset = H5Dcreate2(file_, name.c_str(), dt, dspace, H5P_DEFAULT, prop, H5P_DEFAULT);
  vldatasets_[name] = dset;
  return dset;  
}

void Hdf5Back::AppendVLKey(hid_t dset, DbTypes dbtype, Digest key) {
  hid_t dspace = H5Dget_space(dset);
  hsize_t origlen = H5Sget_simple_extent_npoints(dspace);
  hsize_t newlen[1] = {origlen + 1};
  hsize_t offset[1] = {origlen};
  hsize_t extent[1] = {1};
  hid_t mspace = H5Screate_simple(1, extent, NULL);
  herr_t status = H5Dextend(dset, newlen);
  if (status < 0)
    throw IOError("could not resize key array.");
  dspace = H5Dget_space(dset);
  status = H5Sselect_hyperslab(dspace, H5S_SELECT_SET, offset, NULL, extent, NULL);
  if (status < 0)
    throw IOError("could not select hyperslab of key array.");
  status = H5Dwrite(dset, sha1_type_, mspace, dspace, H5P_DEFAULT, key.val);
  if (status < 0)
    throw IOError("could not write digest to key array.");
  H5Sclose(mspace);
  H5Sclose(dspace);
  vlkeys_[dbtype].insert(key);
}

void Hdf5Back::InsertVLVal(hid_t dset, DbTypes dbtype, Digest key, std::string val) {
  hid_t dspace = H5Dget_space(dset);
  hsize_t extent[CYCLUS_SHA1_NINT] = {1, 1, 1, 1, 1};
  hid_t mspace = H5Screate_simple(CYCLUS_SHA1_NINT, extent, NULL);
  const std::vector<hsize_t> idx = key.cast<hsize_t>();
  herr_t status = H5Sselect_hyperslab(dspace, H5S_SELECT_SET, (const hsize_t*) &idx[0], 
                                      NULL, extent, NULL);
  if (status < 0)
    throw IOError("could not select hyperslab of value array.");
  const char* buf[1] = {val.c_str()};
  status = H5Dwrite(dset, vldts_[dbtype], mspace, dspace, H5P_DEFAULT, buf);
  if (status < 0)
    throw IOError("could not write string to value array.");
  H5Sclose(mspace);
  H5Sclose(dspace);
};

} // namespace cyclus
