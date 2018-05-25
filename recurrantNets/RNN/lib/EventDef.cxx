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
#include "EventDef.h"

ClassImp(EventDef)

//______________________________________________________________________________
EventDef::EventDef() 
 : fnParticles(0)
 , fnParticlesFromX(0)
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
    SetParticleCodes();
    fnParticles      = 0;
    fnParticlesFromX = 0;
}

//______________________________________________________________________________
Int_t EventDef::GetTrackPdg(UInt_t i) const
{
    if (i>=fParticles.size()) return -999;
    return fParticles[i].Pdg;
}

//______________________________________________________________________________
Int_t EventDef::GetTrackMotherPdg(UInt_t i) const
{
    if (i>=fParticles.size()) return -999;
    return GetTrackPdg(fParticles[i].MotherNumber]);
}

//______________________________________________________________________________
Int_t EventDef::GetTrackIsFinal(UInt_t i) const 
{
    if (i>=fParticles.size()) return -999;
    return fParticles[i].isFinal;
}

//______________________________________________________________________________
void EventDef::SortParticles()
{
    std::sort(fParticles.begin(), fParticles.end(), 
              // sorts particles by an ascending Number
              [](const Particle& a, const Particle& b) { 
                    return abs(a.Number) < abs(b.Number); } );
}

//______________________________________________________________________________
void EventDef::AddTrack(Int_t number, Int_t pdg, Int_t mother_number,
        Int_t daugther_nb_1, Int_t daugther_nb_2, Bool_t isFinal)
{
    // add track to next element
    fnParticles++;
    // create the new particle
    Particle new_part;
    new_part.Number = number;
    new_part.Pdg = pdg;
    new_part.MotherPdg = mother_pdg;
    new_part.isFinal = isFinal;
    new_part.DaughterNbs = {daugther_nb_1, daugther_nb_2};
    // and track
    fParticles.push_back(new_part);
    // sort the particles
    SortParticles();
}

//______________________________________________________________________________
Bool_t EventDef::operator==(const EventDef& other) const
{
    if (this->GetnUniqueParticles() != other.GetnUniqueParticles()) return kFALSE;
    for (Int_t ii(0); ii<this->GetnUniqueParticles(); ii++){
        if (fParticles[ii].Pdg != other.GetTrackPdg(ii) ||
            fParticles[ii].MotherPdg != other.GetTrackMotherPdg(ii) ||
            fParticles[ii].isFinal != other.GetTrackIsFinal(ii) ) return kFALSE;  
    }
    return kTRUE;
}

//______________________________________________________________________________
void EventDef::PrintEvent(TString filename) const
{
    std::filebuf fb;
    fb.open(filename.Data(), std::ios::out);
    std::ostream os(&fb);

    TString decayString = GetDecayStringLong();
    os << decayString.Data() << "\n";

    fb.close();
}

//______________________________________________________________________________
TString EventDef::GetDecayStringShort() const
{
    TString decayString("");

    Int_t particle_pdg;
    TString number;
    decayString += "$\\textbf{X} \\to";
    for (UInt_t ii(0); ii<fParticles.size(); ii++){
        if (!fParticles.isFinal) continue;
        particle_pdg = fParticles[ii].Pdg;
        decayString += fParticleCodes.at(particle_pdg);
        if (fParticles[ii].MotherPdg > 10) {
            particle_pdg = fParticles[ii].MotherPdg;
            decayString += "(" + fParticleCodes.at(particle_pdg) + ") ";
        }
    }
    decayString += "$";
    return decayString;
}

//______________________________________________________________________________
TString EventDef::GetDecayStringLong() const
{
    TString decayString("");

    std::vector<Particle> particles_copy = fParticles;
    std::vector<Int_t> eraseIndexVec;

    Int_t particle_pdg;
    TString number;
    decayString += "\\begin{tikzpicture}[dirtree, baseline=(current bounding box.center)]";
    decayString += "\\centering\\node{$X$}";
    while (particles_copy.size()>0) {
        Int_t ii = 0;
        eraseIndexVec.clear();
        if (abs(particles_copy[ii].MotherPdg) < 10) {
            // this means that the particle is a final one
            decayString += " child { node {$" + fParticleCodes.at(particles_copy[ii].Pdg) + "$}";
            if (particles_copy[ii].isFinal) { 
                decayString+="}";
                
            }
            else { 
                /* for */ 
            
            }
            for (UInt_t kk(0); kk<eraseIndexVec.size(); kk++)
                particles_copy.erase(particles_copy.begin() + eraseIndexVec[kk]);
        }
        else ii++;
    }
            

        /* if (fParticles[ii].MotherPdg > 10) { */
        /*     particle_pdg = fParticles[ii].MotherPdg; */
        /*     decayString += "(" + fParticleCodes.at(particle_pdg) + ") "; */
        /* } */
    }
    decayString += "$";
    return decayString;
}

//______________________________________________________________________________
Int_t EventDef::GetTrackIndexInVector(Int_t number) const
{
    for(UInt_t ii(0); ii<fParticles.size(); ii++;){
        if (fParticles[ii].Number == number) return ii; 
    }
    // if that number does not correspond to a particle we return -1
    return -1;
}

//______________________________________________________________________________
Int_t EventDef::TreeLooper(Int_t mother)
{
    if (mother==-1 || mother>=fParticles.size()) return -1;
    printf("Particle: %s, number: %i\n", 
            fParticles[mother].Pdg, mother); 
    if (fParticles[mother].isFinal) return -1;
    for (Int_t ii(fParticles[mother].DaughterNbs[0]); 
            ii<=fParticles[mother].DaughterNbs[1]; ii++) 
    {
        TreeLooper(ii);
    }
    return -1;
}

//______________________________________________________________________________
void EventDef::SetParticleCodes()
{
    fParticleCodes.clear();
    // to correctly print out the backslash we need to put in two backslashes in order for
    // the compiler to identify one backslash
    fParticleCodes[211]  = "\\pi^{+}";
    fParticleCodes[-211] = "\\pi^{-}";
    fParticleCodes[22]   = "\\gamma";
    fParticleCodes[111]  = "\\pi^{0}";
    fParticleCodes[221]  = "\\eta";
    fParticleCodes[331]  = "\\eta\\prime";
    fParticleCodes[223]  = "\\omega";
    fParticleCodes[333]  = "\\phi";
    fParticleCodes[113]  = "\\rho^{0}";
    fParticleCodes[213]  = "\\rho^{+}";
    fParticleCodes[-213] = "\\rho^{-}";
    fParticleCodes[311]  = "\\K^{0}";
    fParticleCodes[310]  = "\\K^{0}_{S}";
    fParticleCodes[130]  = "\\K^{0}_{L}";
    fParticleCodes[321]  = "\\K^{+}";
    fParticleCodes[-321] = "\\K^{-}";
}
