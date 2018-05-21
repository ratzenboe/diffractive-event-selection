/// ////////////////////////////////////////////////////////////////////////////
///
/// structure to hold event information
///
//______________________________________________________________________________
#include <iostream>
#include <algorithm>

#include "EventDef.h"

ClassImp(EventDef)

//______________________________________________________________________________
EventDef::EventDef()
{
    this->Reset();
}

//______________________________________________________________________________
EventDef::~EventDef()
{

}

//______________________________________________________________________________
void EventDef::Reset()
{
    // reset all private variables
    fParticles.clear();
}

//______________________________________________________________________________
Int_t EventDef::TrackPdg(UInt_t i) const
{
    if (i>=fParticles.size()) return -999;
    return fParticles[i].Pdg;
}

//______________________________________________________________________________
Int_t EventDef::TrackOccurance(UInt_t i) const 
{
    if (i>=fParticles.size()) return -999;
    return fParticles[i].Occurance;
}

//______________________________________________________________________________
Int_t EventDef::TrackMotherPdg(UInt_t i) const
{
    if (i>=fParticles.size()) return -999;
    return fParticles[i].MotherPdg;
}

//______________________________________________________________________________
void EventDef::SortParticles()
{
    std::sort(fParticles.begin(), fParticles.end(), 
              [](const Particle& a, const Particle& b) { return a.Pdg < b.Pdg; } );
}

//______________________________________________________________________________
void EventDef::AddTrack(Int_t pdg, Int_t mother)
{
    // add track to next element
    Bool_t hasDuplicate = kFALSE;
    for (UInt_t ii(0); ii<fParticles.size(); ii++){
        if (fParticles[ii].Pdg == pdg && fParticles[ii].MotherPdg == mother) {
            fParticles[ii].Occurance++;
            hasDuplicate = kTRUE;
        }
    }
    if (!hasDuplicate) {
        Particle new_part;
        new_part.Pdg = pdg;
        new_part.MotherPdg = mother;
        new_part.Occurance = 1;
    }
    SortParticles();
}

//______________________________________________________________________________
Bool_t EventDef::operator==(const EventDef& other) const
{
    if (this->nParticles() != other.nParticles()) return kFALSE;
    for (Int_t ii(0); ii<this->nParticles(); ii++){
        if (fParticles[ii].Pdg != other.TrackPdg(ii) ||
            fParticles[ii].MotherPdg != other.TrackMotherPdg(ii) ||
            fParticles[ii].Occurance != other.TrackOccurance(ii)) return kFALSE;  
    }
    return kTRUE;
}

//______________________________________________________________________________



