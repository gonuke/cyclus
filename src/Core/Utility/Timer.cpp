// Timer.cpp
// Implements the Timer class

#include "Timer.h"

#include <string>
#include <iostream>

#include "InputXML.h"
#include "CycException.h"
#include "Logger.h"
#include "Material.h"

using namespace std;

Timer* Timer::instance_ = 0;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Timer::runSim() {
  CLOG(LEV_INFO1) << "Simulation set to run from start="
                  << startDate_ << " to end=" << endDate_;

  time_ = -1;
  handlePreHistory();
  time_ = time0_;
  CLOG(LEV_INFO1) << "Beginning simulation";
  while (date_ < endDate()){
    if (date_.day() == 1){
      CLOG(LEV_INFO2) << "Current date: " << date_ << " Current time: " << time_ << " {";
      CLOG(LEV_DEBUG3) << "The list of current tick listeners is: " << reportListeners();

      Material::decayMaterials(time_);
      sendTick();
      sendResolve();
    }
    
    int eom_day = lastDayOfMonth();
    for (int i = 1; i < eom_day+1; i++){
      sendDailyTasks();
      if (i == eom_day){
        sendTock();
        CLOG(LEV_INFO2) << "}";
      }
      date_ += boost::gregorian::days(1);
    }

    time_++;
  }

  // initiate deletion of models that don't have parents.
  // dealloc will propogate through hierarchy as models delete their children
  int count = 0;
  while (Model::getModelList().size() > 0) {
    Model* model = Model::getModelList().at(count);
    try {
      model->parent();
    } catch (CycIndexException err) {
      delete model;
      count = 0;
      continue;
    }
    count++;
  }
}

int Timer::lastDayOfMonth(){
  int lastDay = 
    boost::gregorian::gregorian_calendar::end_of_month_day(date_.year(),
							   date_.month());
  return lastDay;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string Timer::reportListeners() {
  string report = "";
  for(vector<TimeAgent*>::iterator agent=tick_listeners_.begin();
      agent != tick_listeners_.end(); 
      agent++) {
    report += (*agent)->name() + " ";
  }
  return report;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Timer::handlePreHistory() {
  for(vector<TimeAgent*>::iterator agent=tick_listeners_.begin();
       agent != tick_listeners_.end(); 
       agent++) {
    try {
      (*agent)->handlePreHistory();
    } catch(CycException err) {
      LOG(LEV_ERROR, "none!") << "ERROR occured in handlePreHistory(): " << err.what();
    }
  }
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Timer::sendResolve() {
  for(vector<MarketModel*>::iterator agent=resolve_listeners_.begin();
       agent != resolve_listeners_.end(); 
       agent++) {
    try {
      CLOG(LEV_INFO3) << "Sending resolve to Model ID=" << (*agent)->ID()
                      << ", name=" << (*agent)->name() << " {";
      (*agent)->resolve();
    } catch(CycException err) {
      CLOG(LEV_ERROR) << "ERROR occured in sendResolve(): " << err.what();
    }
    CLOG(LEV_INFO3) << "}";
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Timer::sendTick() {
  for(vector<TimeAgent*>::iterator agent=tick_listeners_.begin();
       agent != tick_listeners_.end(); 
       agent++) {
    try {
      CLOG(LEV_INFO3) << "Sending tick to Model ID=" << (*agent)->ID()
                      << ", name=" << (*agent)->name() << " {";
      (*agent)->handleTick(time_);
    } catch(CycException err) {
      CLOG(LEV_ERROR) << "ERROR occured in sendTick(): " << err.what();
    }
    CLOG(LEV_INFO3) << "}";
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Timer::sendTock() {
  for(vector<TimeAgent*>::iterator agent=tick_listeners_.begin();
       agent != tick_listeners_.end(); 
       agent++) {
    try {
      CLOG(LEV_INFO3) << "Sending tock to Model ID=" << (*agent)->ID()
                      << ", name=" << (*agent)->name() << " {";
      (*agent)->handleTock(time_);
    } catch(CycException err) {
      CLOG(LEV_ERROR) << "ERROR occured in sendTock(): " << err.what();
    }
    CLOG(LEV_INFO3) << "}";
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Timer::sendDailyTasks() {
  for(vector<TimeAgent*>::iterator agent=tick_listeners_.begin();
       agent != tick_listeners_.end(); 
       agent++) {
    try {
      (*agent)->handleDailyTasks(time_,date_.day());
    } catch(CycException err) {
      CLOG(LEV_ERROR) << "ERROR occured in sendDailyTasks(): " << err.what();
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Timer::registerTickListener(TimeAgent* agent) {
  CLOG(LEV_INFO2) << "Model ID=" << agent->ID() << ", name=" << agent->name()
                  << " has registered to receive 'ticks' and 'tocks'.";
  tick_listeners_.push_back(agent);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Timer::registerResolveListener(MarketModel* agent) {
  CLOG(LEV_INFO2) << "Model ID=" << agent->ID() << ", name=" << agent->name()
                  << " has registered to receive 'resolves'.";
  resolve_listeners_.push_back(agent);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Timer::time() {
  return time_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Timer* Timer::Instance() {
  // If we haven't created a Timer yet, create it, and then and return it
  // either way.
  if (0 == instance_) {
    instance_ = new Timer();
    instance_->initialize();
  }

  return instance_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Timer::initialize(std::string xmlParams) {

  XMLQueryEngine xqe(xmlParams);

  // get duration
  simDur_ = strtol(xqe.get_contents("duration").c_str(),NULL,10);

  // get simulation start
  time0_ = strtol(xqe.get_contents("simstart").c_str(),NULL,10);
  time_ = time0_;

  // get start month
  month0_ = strtol(xqe.get_contents("startmonth").c_str(),NULL,10);
  if (month0_ < 1 || month0_ > 12)
    throw CycRangeException("Invalid month0; must be between 1 and 12 (inclusive).");

  // get start year  
  year0_ = strtol(xqe.get_contents("startyear").c_str(),NULL,10);
  if (year0_ < 1942)
    throw CycRangeException("Invalid year0; the first man-made nuclear reactor was build in 1942");

  if (year0_ > 2063)
    throw CycRangeException("Invalid year0; why start a simulation after we've got warp drive?: http://en.wikipedia.org/wiki/Warp_drive#Development_of_the_backstory");

  // get decay interval
  int decay = strtol(xqe.get_contents("decay").c_str(),NULL,10);
  if (decay > dur)
    throw CycRangeException("Invalid decay interval; no decay occurs if the interval is greater than the simulation duriation. For no decay, use -1 .");
  Material::setDecay(decay);

  startDate_ = boost::gregorian::date(year0_,month0_,1);
  endDate_ = getEndDate(startDate_,simDur_);
  date_ = boost::gregorian::date(startDate_);

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
boost::gregorian::date Timer::getEndDate(boost::gregorian::date startDate, int simDur_){
  boost::gregorian::date endDate(startDate);
  endDate += boost::gregorian::months(simDur_-1);
  endDate += boost::gregorian::days(boost::gregorian::gregorian_calendar::end_of_month_day(endDate.year(),endDate.month())-1);
  return endDate;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Timer::simDur() {
  return simDur_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Timer::Timer() {
  time_ = 0;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Timer::convertDate(int month, int year) {
  return (year - year0_) * 12 + (month - month0_) + time0_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
pair<int, int> Timer::convertDate(int time) {
  int month = (time - time0_) % 12 + 1;
  int year = (time - time0_ - (month - 1)) / 12 + year0_;
  return make_pair(month, year);
}



