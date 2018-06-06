/// ////////////////////////////////////////////////////////////////////////////
///
/// structure to hold event information
///
//______________________________________________________________________________
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <algorithm>

#include <TDatabasePDG.h>
#include "EventStorage.h"

ClassImp(EventStorage)

//______________________________________________________________________________
EventStorage::EventStorage() 
 /* : EventDef() */
 : fnEvents(0)
{
    this->Reset();
}

//______________________________________________________________________________
EventStorage::~EventStorage()
{

}

//______________________________________________________________________________
void EventStorage::Reset()
{
    // reset all private variables
    fEvents.clear();
    fnEvents = 0;
}

//______________________________________________________________________________
void EventStorage::AddEvent(EventDef event)
{
    // the stored event should be finalized to properly print the event
    if (!event.GetIsFinalized()) event.FinalizeEvent();
    fnEvents++;
    // check if event already exists, the == operator of EventDef is overloaded:
    for (UInt_t ii(0); ii<fEvents.size(); ii++) {
        if (fEvents[ii] == event) { fEvents[ii].IncreaseDecayOccurance(); return ; } 
    }
    // a new event-class: add it to the event vector
    fEvents.push_back(event);
    SortEvents();
}

//______________________________________________________________________________
void EventStorage::SortEvents()
{
    std::sort(fEvents.begin(), fEvents.end(), 
              // sorts events by occurance and number of particles
              [](const EventDef& a, const EventDef& b) { 
                    return (a.GetOccurance()==b.GetOccurance()) ? 
                       (a.GetnParticles()<b.GetnParticles()) : (a.GetOccurance()>b.GetOccurance()); 
              } );
}

//______________________________________________________________________________
void EventStorage::PrintNEvts(TString filename, Int_t nb) const
{
    std::filebuf fb;
    fb.open(filename.Data(), std::ios::out);
    std::ostream os(&fb);

    TString beginTableStr = "\\begin{center}\\setlength{\\tabcolsep}{9mm}\\def";
    beginTableStr += "\\arraystretch{1.25}\\centering\\begin{longtable}{l | c} "; 
    beginTableStr += "Decay & Occurance[\\%] \\\\ \\hline \\hline";
    TString endTableStr = "\\end{longtable}\\end{center}";
 
    TString decayString;
    nb = (abs(nb)>=fEvents.size() || nb==-1) ? fEvents.size() : abs(nb);
    os << beginTableStr.Data() << "\n";
    Int_t countChilds(0);
    for (Int_t ii(0); ii<nb; ii++) {
        decayString = fEvents[ii].GetDecayStringLong(fnEvents);
        printf("%i/%i events of:\n%s\n----------------------------------------\n", fEvents[ii].GetOccurance(), fnEvents, decayString.Data());
        os << decayString.Data() << "\n";
    }
    os << endTableStr.Data() << "\n";

    fb.close();
}

//______________________________________________________________________________
