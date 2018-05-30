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
EventDef::EventDef(Int_t rootpdg, TString rootstr) 
 : fnParticles(0)
 , fDecayOccurance(1)
 , fIsFinalized(kFALSE)
 , fRootPDG(rootpdg)
 , fRootString(std::string(rootstr.Data()))
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
    fnParticles = 0;
    fDecayOccurance = 1;
    fIsFinalized = kFALSE;
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
    return GetTrackPdg(fParticles[i].MotherNumber);
}

//______________________________________________________________________________
Int_t EventDef::GetTrackIsFinal(UInt_t i) const 
{
    if (i>=fParticles.size()) return -999;
    return fParticles[i].isFinal;
}

//______________________________________________________________________________
Int_t EventDef::GetTrackOccurance(UInt_t i) const 
{
    if (i>=fParticles.size()) return -999;
    return fParticles[i].PartOccurance;
}

//______________________________________________________________________________
void EventDef::SortParticles()
{
    std::sort(fParticles.begin(), fParticles.end(), 
              // sorts particles by an ascending pdg value (important for compaison)
              [](const Particle& a, const Particle& b) { 
                    return (abs(a.Pdg) == abs(b.Pdg)) ? (a.Pdg<b.Pdg) : (abs(a.Pdg)<abs(b.Pdg)); 
              } );
}

//______________________________________________________________________________
void EventDef::AddTrack(Int_t number, Int_t pdg, Int_t mother_number,
        Int_t daugther_nb_1, Int_t daugther_nb_2, Bool_t isFinal)
{
    if (fIsFinalized) { printf("<W> event is already finalized! Track is not added!"); return ; }
    // add track to next element
    fnParticles++;
    // create the new particle
    Particle new_part;
    new_part.Number = number;
    new_part.MotherNumber = mother_number;
    new_part.Pdg = pdg;
    new_part.isFinal = isFinal;
    new_part.PartOccurance = 1;
    new_part.DaughterVec.clear();
    for (Int_t ii(daugther_nb_1); ii<=daugther_nb_2; ii++) new_part.DaughterVec.push_back(ii);
    // and track
    fParticles.push_back(new_part);
}

//______________________________________________________________________________
void EventDef::FinalizeEvent()
{
    std::vector<Int_t> eraseVec, daughtersOfX;
    Int_t Xnumber(-1);
    eraseVec.clear();
    daughtersOfX.clear();
    // C++11 loop-style
    printf("DEBUG: first loop\n");
    for (Particle part : fParticles) {
        if (abs(part.Pdg) < 10 || part.Pdg == 21) { eraseVec.push_back(part.Number); continue; }
        // we rewrite the initial system X to number 1
        if (part.Pdg==fRootPDG) { Xnumber = part.Number; continue; }
        // this means that every particle X decays into has to have mother 1:
        if (IsFromX(part.Number)) { daughtersOfX.push_back(part.Number); }
    }
    // now set the daugthers of X to the correct daughters-vector
    for (Int_t nb : daughtersOfX) fParticles[GetParticleIndexFromNumber(nb)].MotherNumber = 1;
    fParticles[GetParticleIndexFromNumber(Xnumber)].Number = 1;
    fParticles[GetParticleIndexFromNumber(1)].DaughterVec = daughtersOfX;
    // erase the gluons and quarks
    for (Int_t nb : eraseVec) {
          Int_t element = GetParticleIndexFromNumber(nb);
          fParticles.erase(fParticles.begin()+element);
    }
    printf("DEBUG: sort particles\n");
    SortParticles();
    // loop again through the particle list and combine same decay-particles 
    // works only on a pre-sorted vector
    ////////////////////////////////////////////////////////////////////////////////////////
    // redo this function!!!!!!!!!!!
    /* printf("DEBUG: unique parts\n"); */
    /* std::vector<Particle>::iterator it = std::unique(fParticles.begin(), fParticles.end(), */ 
    /*         [this](Particle& a, const Particle& b) { */ 
    /*             if (a==b && a.isFinal && b.isFinal){ */
    /*                 // increase the particle occurance of the first particle */
    /*                 a.PartOccurance++; */
    /*                 // delete the second particle from the mothers daugthervector */
    /*                 fParticles[GetParticleIndexFromNumber(b.MotherNumber)].DaughterVec.erase( */
    /*                   std::find( */
    /*                     fParticles[GetParticleIndexFromNumber(b.MotherNumber)].DaughterVec.begin(), */                        
    /*                     fParticles[GetParticleIndexFromNumber(b.MotherNumber)].DaughterVec.end(), */ 
    /*                     b.Number)); */
    /*                 return kTRUE; */
    /*             } else return kFALSE; */
    /*         } ); */
    /* printf("DEBUG: resize particles\n"); */
    /* fParticles.resize( std::distance(fParticles.begin(),it) ); */

    printf("DEBUG: sort particles II\n");
    SortParticles();
    printf("DEBUG: order daugthers\n");
    OrderDaugthers();
    // just to check if everything worked
    for (Particle part : fParticles) {
        printf("Number: %i, pdg: %i, mothernumber: %i, isFinal: %i, PartOccurance: %i",
                part.Number, part.Pdg, part.MotherNumber,part.isFinal, part.PartOccurance);
        printf(", DaughterNumbers: ");
        for (Int_t nb : part.DaughterVec) printf("%i ", nb);
        printf("\n");
    }
    fIsFinalized = kTRUE;
}

//______________________________________________________________________________
Bool_t EventDef::IsFromX(Int_t number) const
{
    Int_t motherNumber = fParticles[GetParticleIndexFromNumber(number)].MotherNumber;
    Int_t motherPdg    = fParticles[GetParticleIndexFromNumber(motherNumber)].Pdg;
    // loopexiter:
    Int_t prevMotherNumber;
    // sign of the X particle: pdg==9900110 -> if X is parent
    while (kTRUE)
    {   // if parent is gluon or quark we search for the next parent
        prevMotherNumber = motherNumber;
        if (motherPdg==fRootPDG) return kTRUE;
        if (abs(motherPdg)<10 || motherPdg==21) {
            motherNumber = fParticles[GetParticleIndexFromNumber(motherNumber)].MotherNumber;     
            motherPdg    = fParticles[GetParticleIndexFromNumber(motherNumber)].Pdg;     
        } else return kFALSE;
        if (prevMotherNumber==motherNumber) break;
    }
    return kFALSE;
}

//______________________________________________________________________________
Bool_t EventDef::operator==(const EventDef& other) const
{
    if (this->GetnUniqueParticles() != other.GetnUniqueParticles()) return kFALSE;
    for (Int_t ii(0); ii<this->GetnUniqueParticles(); ii++){
        if (fParticles[ii].Pdg != other.GetTrackPdg(ii) ||
            this->GetTrackMotherPdg(ii) != other.GetTrackMotherPdg(ii) ||
            fParticles[ii].isFinal != other.GetTrackIsFinal(ii) || 
            fParticles[ii].PartOccurance != other.GetTrackOccurance(ii)) return kFALSE;  
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
        if (!fParticles[ii].isFinal) continue;
        particle_pdg = fParticles[ii].Pdg;
        decayString += fParticleCodes.at(particle_pdg);
        if (GetTrackMotherPdg(ii) > 10) {
            particle_pdg = GetTrackMotherPdg(ii);
            decayString += "(" + fParticleCodes.at(particle_pdg) + ") ";
        }
    }
    decayString += "$";
    return decayString;
}

//______________________________________________________________________________
TString EventDef::GetDecayStringLong(Int_t nEvts) const
{
    TString decayString("");
    TString numberString;
    numberString.Form("%.2f", Double_t(fDecayOccurance)/Double_t(nEvts)*100.);

    Int_t particle_pdg;
    TString number;
    decayString += "\\begin{tikzpicture}[dirtree, baseline=(current bounding box.center)]";
    decayString += "\\centering";
    TreeLooper(1, decayString); 
    decayString += ";";
    decayString += "\\addvmargin{1mm}\\end{tikzpicture}  & " + numberString;
    decayString += "\\\\ \\hline";
    return decayString;
}

//______________________________________________________________________________
Int_t EventDef::GetParticleIndexFromNumber(Int_t number) const
{
    for(UInt_t ii(0); ii<fParticles.size(); ii++) {
        if (fParticles[ii].Number == number) return ii; 
    }
    // if that number does not correspond to a particle we return -1
    return -1;
}

//______________________________________________________________________________
Int_t EventDef::TreeLooper(Int_t mother, TString& decaystring) const
{
    mother = GetParticleIndexFromNumber(mother);
    if (mother==-1 || mother>=(Int_t)fParticles.size()) return -1; 
    // formulate the string
    if ( fParticles[mother].Pdg == fRootPDG ) 
        decaystring += "\\node{$" + fParticleCodes.at(fRootPDG) + "$} ";
    else if ( fParticleCodes.find(fParticles[mother].Pdg) != fParticleCodes.end()) {
        decaystring += "child { node {$"; 
        if (fParticles[mother].PartOccurance>1) {
            TString particleOccurance;
            particleOccurance.Form("%i",fParticles[mother].PartOccurance);
            decaystring += particleOccurance;
        }
        decaystring += fParticleCodes.at(fParticles[mother].Pdg);
        decaystring += "$} ";
    } else {
        printf("Pdg value %i is not contained in the particle-list!\n",fParticles[mother].Pdg);
    }
    if (fParticleCodes.find(fParticles[mother].Pdg) != fParticleCodes.end() && 
            fParticles[mother].isFinal) { decaystring += "} "; return -1; }
    // C++11 loop style
    for (Int_t it : fParticles[mother].DaughterVec)
    {
        TreeLooper(it, decaystring);
    }
    if ( fParticleCodes.find(fParticles[mother].Pdg) != fParticleCodes.end() && 
            fParticles[mother].Pdg != fRootPDG ) decaystring+="} ";
    return -1;
}

//______________________________________________________________________________
Int_t EventDef::OrderDaugthers(Int_t mother)
{
    mother = GetParticleIndexFromNumber(mother);
    if (mother==-1 || mother>=(Int_t)fParticles.size() || fParticles[mother].isFinal) return -1; 
    // here: order the daugthers by pdg value
    std::sort(fParticles[mother].DaughterVec.begin(), fParticles[mother].DaughterVec.end(),
            // sort daugthers 
            [this](const Int_t& a, const Int_t& b) 
            {
                Int_t a_DaugtherPdg = fParticles[GetParticleIndexFromNumber(a)].Pdg;
                Int_t b_DaugtherPdg = fParticles[GetParticleIndexFromNumber(b)].Pdg;
                if (abs(a_DaugtherPdg)==abs(b_DaugtherPdg)) return a_DaugtherPdg>b_DaugtherPdg;
                else return abs(a_DaugtherPdg)<abs(b_DaugtherPdg);
            });
    // C++11 loop style
    for (Int_t it : fParticles[mother].DaughterVec)
    {
        OrderDaugthers(it);
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
    fParticleCodes[311]  = "K^{0}";
    fParticleCodes[310]  = "K^{0}_{S}";
    fParticleCodes[130]  = "K^{0}_{L}";
    fParticleCodes[321]  = "K^{+}";
    fParticleCodes[-321] = "K^{-}";
    fParticleCodes[fRootPDG] = fRootString;
}
