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
        if (event == fEvents[ii]) { fEvents[ii].IncreaseDecayOccurance(); return ; } 
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
                    if (a.GetOccurance()==b.GetOccurance()) 
                        return (a.GetnParticles()<b.GetnParticles());
                    else return (a.GetOccurance()>b.GetOccurance());
                    /* return (a.GetOccurance()==b.GetOccurance()) ? */ 
                    /*    (a.GetnParticles()<b.GetnParticles()) : (a.GetOccurance()>=b.GetOccurance()); */ 
              } );
}

//______________________________________________________________________________
void EventStorage::PrintNEvts(TString filename, Int_t nb) const
{
    std::filebuf fb;
    fb.open(filename.Data(), std::ios::out);
    std::ostream os(&fb);

    TString beginTableStr = "\\begin{center}\\setlength{\\tabcolsep}{3mm}\\def";
    beginTableStr += "\\arraystretch{1.25}\\centering\\begin{longtable}{l | c | c } "; 
    beginTableStr += "Decay & Occurance[\\%] & Cumulative [\\%] \\\\ \\hline \\hline";
    TString endTableStr = "\\end{longtable}\\end{center}";
 
    TString decayString;
    TString numberString;
    TString cumulativeStr;
    TString emcal_cumul_str;
    TString gt_two_pion_cumul_str;
    Int_t total_evts = 0;
    Int_t emcal_cumul_int = 0;
    Int_t gt_two_pion_int = 0;

    nb = (abs(nb)>=fEvents.size() || nb==-1) ? fEvents.size() : abs(nb);
    os << beginTableStr.Data() << "\n";
    for (Int_t ii(0); ii<nb; ii++) {
        decayString = fEvents[ii].GetDecayStringLong();
        
        numberString.Form("%.2f", Double_t(fEvents[ii].GetOccurance())/Double_t(fnEvents)*100.);
        total_evts += fEvents[ii].GetOccurance();
        cumulativeStr.Form("%.2f", Double_t(total_evts)/Double_t(fnEvents)*100.);
        // bg classification
        if (fEvents[ii].IsEMCALCase()) { 
            os << "\\rowcolor{LightRed}"; 
            emcal_cumul_int += fEvents[ii].GetOccurance(); 
        }
        else if (fEvents[ii].IsThreePlusCase()) {
            os << "\\rowcolor{LightCyan}";
            gt_two_pion_int += fEvents[ii].GetOccurance(); 
        }
        emcal_cumul_str.Form("%.2f", Double_t(emcal_cumul_int)/Double_t(fnEvents)*100.);
        gt_two_pion_cumul_str.Form("%.2f", Double_t(gt_two_pion_int)/Double_t(fnEvents)*100.);
        // outstream
        os << decayString.Data(); 
        os << " & " << numberString.Data() << " & " << cumulativeStr.Data();
        /* os << " & " << emcal_cumul_str.Data() << " & " << gt_two_pion_cumul_str.Data(); */ 
        os << "\\\\ \\hline";
        os << "\n";
        // we want to limit ourselves to describing 95% of the events to not deal with huge files
        if (Double_t(total_evts)/Double_t(fnEvents)*100. > 95.) break;
    }
    os << endTableStr.Data() << "\n";

    fb.close();
}

