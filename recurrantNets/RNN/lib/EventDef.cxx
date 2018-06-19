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
    return fParticles[i].MotherPdg;
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
              // sorts particles by an ascending pdg value (important for compaison)
              [](const Particle& a, const Particle& b) { 
                    if (abs(a.Pdg) == abs(b.Pdg)) {
                        if (a.Pdg == b.Pdg) return (a.MotherPdg<b.MotherPdg);
                        else return (a.Pdg<b.Pdg);
                    }
                    else return (abs(a.Pdg)<abs(b.Pdg));
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
    new_part.MotherPdg = -1;
    new_part.Pdg = pdg;
    new_part.isFinal = isFinal;
    new_part.DaughterVec.clear();
    for (Int_t ii(daugther_nb_1); ii<=daugther_nb_2; ii++) new_part.DaughterVec.push_back(ii);
    // and track
    fParticles.push_back(new_part);
}

//______________________________________________________________________________
void EventDef::FillMotherPdg()
{
    if (fIsFinalized) { printf("<W> event is already finalized! Track is not added!"); return ; }
    // add track to next element
    for (UInt_t ii(0); ii<fParticles.size(); ii++)
    {
        // get mother particle 
        Int_t mother_number = fParticles[ii].MotherNumber;
        Int_t mother_idx_in_part_vec = GetParticleIndexFromNumber(mother_number);

        // if the mother-idx is not in the list, the particle is the incident X particle
        if (mother_idx_in_part_vec==-1) {
            // the incident X particle get a mother pdg value of -1
            fParticles[ii].MotherPdg = -1; 
        } else {
            fParticles[ii].MotherPdg = fParticles[mother_idx_in_part_vec].Pdg;
        }
    }
    return ;
}

//______________________________________________________________________________
void EventDef::FinalizeEvent()
{
    std::vector<Int_t> eraseVec, daughtersOfX;
    Int_t Xnumber(-1);
    eraseVec.clear();
    daughtersOfX.clear();
    // C++11 loop-style
    /* printf("DEBUG: first loop\n"); */
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
    SortParticles();
    OrderDaugthers();

    FillMotherPdg();
    fIsFinalized = kTRUE;
}

void EventDef::PrintStack() const
{
    for (Particle part : fParticles)
    {
        printf("Nr: %i, Pdg: %i, MotherPdg: %i, final: %s", 
                part.Number, part.Pdg, part.MotherPdg, (part.isFinal) ? "true" : "false");
        printf(", daugthers: ");
        for (Int_t daugther_nb : part.DaughterVec) printf("%i ", daugther_nb);
        printf("\n");
    }
    return ; 
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
TString EventDef::GetDecayStringLong() const
{
    TString decayString("");

    Int_t particle_pdg;
    TString number;
    decayString += "\\begin{tikzpicture}[dirtree, baseline=(current bounding box.center)]";
    decayString += "\\centering";
    TreeLooper(1, decayString); 
    decayString += ";";
    decayString += "\\addvmargin{1mm}\\end{tikzpicture}"; //  & " + numberString;
    /* decayString += "\\\\ \\hline"; */
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
Int_t EventDef::GetParticleIndexFromNumber(Int_t number, std::vector<Particle> particle_vec) const
{
    for(UInt_t ii(0); ii<particle_vec.size(); ii++) {
        if (particle_vec[ii].Number == number) return ii; 
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
        decaystring += fParticleCodes.at(fParticles[mother].Pdg);
        decaystring += "$} ";
    } else {
        printf("Pdg value %i is not contained in the particle-list!\n",fParticles[mother].Pdg);
    }
    // if the particle is final and its pdg is in the fParticleCodes we append a "}"
    if (fParticleCodes.find(fParticles[mother].Pdg) != fParticleCodes.end() && 
            fParticles[mother].isFinal) { decaystring += "} "; return -1; }
 
    if (AllDaughtersFinal(fParticles[mother].Number)) {
        decaystring += "child { node {$"; 
        for (Int_t it : fParticles[mother].DaughterVec) {
            Int_t pdg_daugther = fParticles[GetParticleIndexFromNumber(it)].Pdg;
            if (fParticleCodes.find(pdg_daugther) != fParticleCodes.end()) {
                decaystring += fParticleCodes.at(fParticles[GetParticleIndexFromNumber(it)].Pdg);
            } else { 
                printf("Pdg value %i not contained in particle-list!\n", pdg_daugther);
            }
        }
        // need 2 closing parentesis as we close child and node
        decaystring += "$} } ";
    } else {
        for (Int_t it : fParticles[mother].DaughterVec)
        {
            TreeLooper(it, decaystring);
        }
    }


    // if all daugthers a final we just write them next to each other 
    // C++11 loop style
    if ( fParticleCodes.find(fParticles[mother].Pdg) != fParticleCodes.end() && 
            fParticles[mother].Pdg != fRootPDG ) decaystring+="} ";
    return -1;
}

//______________________________________________________________________________
Bool_t EventDef::operator==(const EventDef& other) const
{
    std::vector<Particle> all_particles_1 = fParticles;
    std::vector<Particle> all_particles_2 = other.GetParticleVec();
    /* if (this->GetnUniqueParticles() != other.GetnUniqueParticles()) return kFALSE;
    for (Int_t ii(0); ii<this->GetnUniqueParticles(); ii++){
        if (abs(fParticles[ii].Pdg)       != abs(other.GetTrackPdg(ii))       ||
            abs(fParticles[ii].MotherPdg) != abs(other.GetTrackMotherPdg(ii)) ||
            fParticles[ii].isFinal        != other.GetTrackIsFinal(ii)) return kFALSE;  
    }
    */
    Bool_t isSame = kTRUE;
    isSame = AreIdentical(this->GetXParticle(), other.GetXParticle(), 
                 all_particles_1, all_particles_2, isSame);
    return isSame;
}

//______________________________________________________________________________
EventDef::Particle EventDef::GetXParticle() const
{
    for (Particle part : fParticles) if (part.Pdg == fRootPDG) return part;
    printf("<W> root particle not found in fParticles vector, returning first particle in vec\n");
    return fParticles[0]; 
}

//______________________________________________________________________________
Bool_t EventDef::AreIdentical(EventDef::Particle part1, EventDef::Particle part2, std::vector<Particle> all_particles_1, std::vector<Particle> all_particles_2, Bool_t& isSame) const
{
    if (abs(part1.Pdg)!=abs(part2.Pdg)) { isSame = isSame & kFALSE; return kFALSE; }
    /*1. both empty */
    if (part1.isFinal  && part2.isFinal) { isSame = isSame & kTRUE; return kTRUE; }
 
    /* 2. both non-empty -> compare them */
    if (!part1.isFinal  && !part2.isFinal)
    {
        if (part1.DaughterVec.size()!=part2.DaughterVec.size()){ 
            isSame = isSame & kFALSE; return kFALSE; 
        }
        for (UInt_t dIdx(0); dIdx<part1.DaughterVec.size(); dIdx++)
        { 
            Particle daughter1 = all_particles_1[
                GetParticleIndexFromNumber(part1.DaughterVec[dIdx], all_particles_1)];
            Particle daughter2 = all_particles_2[
                GetParticleIndexFromNumber(part2.DaughterVec[dIdx], all_particles_2)];
            isSame = isSame & AreIdentical(daughter1,daughter2,
                                           all_particles_1,all_particles_2,
                                           isSame);
        }
        return isSame;
    }
    /* 3. one empty, one not -> false */
    return kFALSE;
}

//______________________________________________________________________________
Bool_t EventDef::AllDaughtersFinal(Int_t mother_number) const
{
    Bool_t allFinal = kTRUE;
    for (Int_t daugther_nb : fParticles[GetParticleIndexFromNumber(mother_number)].DaughterVec)
    {
        allFinal = allFinal & fParticles[GetParticleIndexFromNumber(daugther_nb)].isFinal;
    }
    return allFinal;
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
                if (abs(a_DaugtherPdg)==abs(b_DaugtherPdg)) {
                    Int_t counter_a(0), counter_b(0);
                    CountDaughterPdgCodes(a, counter_a);
                    CountDaughterPdgCodes(b, counter_b);
                    // if the particles are the same we want first the positive then the neg. one
                    if (counter_a == counter_b) return (a_DaugtherPdg > b_DaugtherPdg);
                    else return (counter_a<counter_b);
                }
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
Bool_t EventDef::CountDaughterPdgCodes(Int_t daughterNumber, Int_t &counter) const
{
    daughterNumber = GetParticleIndexFromNumber(daughterNumber);
    counter += abs(fParticles[daughterNumber].Pdg);
    if (fParticles[daughterNumber].isFinal) return kTRUE;
    for (Int_t it : fParticles[daughterNumber].DaughterVec)
    {
        CountDaughterPdgCodes(it, counter);
    }
    return kTRUE; 
}

//______________________________________________________________________________
Bool_t EventDef::IsEMCALCase() const
{
    Int_t nPions(0), nGammas(0), nElse(0);
    for (Particle part : fParticles)
    {
        if (!part.isFinal) continue;
        // check finals
        Double_t charge = TDatabasePDG::Instance()->GetParticle(part.Pdg)->Charge();
        if (part.Pdg==22) nGammas++;
        else if (abs(part.Pdg)==211) nPions++;
        else if (charge!=0.) nElse++;
    }
    if (nElse==0 && nGammas>0 && nPions==2) return kTRUE;
    else return kFALSE;
}

//______________________________________________________________________________
Bool_t EventDef::IsThreePlusCase() const
{
    Int_t nPions(0), nChargedElse(0);
    for (Particle part : fParticles)
    {
        if (!part.isFinal) continue;
        // check finals
        Double_t charge = TDatabasePDG::Instance()->GetParticle(part.Pdg)->Charge();
        if (abs(part.Pdg)==211) nPions++;
        else if (charge!=0.) nChargedElse++;
    }
    if (nPions>2) return kTRUE;
    else if (nPions>=2 && nChargedElse>0) return kTRUE;
    else return kFALSE;
}

//______________________________________________________________________________
Bool_t EventDef::HasGamma() const
{
    for (Particle part : fParticles)
    {
        if (!part.isFinal) continue;
        // check finals
        if (part.Pdg==22) return kTRUE;
    }
    return kFALSE;
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

    fParticleCodes[-313] = "\\overline{K^{\\ast 0}}";
    fParticleCodes[313] = "K^{\\ast 0}";
    fParticleCodes[323] = "K^{\\ast +}";
    fParticleCodes[-323] = "K^{\\ast -}";
    fParticleCodes[2112] = "N";
    fParticleCodes[-2112] = "\\overline{N}";
    fParticleCodes[2224] = "\\Delta^{++}";
    fParticleCodes[2214] = "\\Delta^{+}";
    fParticleCodes[2114] = "\\Delta^{0}";
    fParticleCodes[1114] = "\\Delta^{-}";
    fParticleCodes[2212] = "P";
    fParticleCodes[-2212] = "\\overline{P}";

    fParticleCodes[3112]  = "\\Sigma^{-}";
    fParticleCodes[-3112] = "\\overline{\\Sigma^{-}}";
    fParticleCodes[11]    = "e^{-}";
    fParticleCodes[-11]   = "e^{+}";
    fParticleCodes[311]   = "K^{0}";
    fParticleCodes[-311]  = "\\overline{K^{0}}";
    fParticleCodes[3114]  = "\\Sigma^{\\ast -}";
    fParticleCodes[-3114] = "\\overline{\\Sigma^{\\ast -}}";
    fParticleCodes[-2224] = "\\overline{\\Delta^{++}}";
    fParticleCodes[990]   = "XXX";
    fParticleCodes[3122]  = "\\Lambda";
    fParticleCodes[-3122] = "\\overline{\\Lambda}";
    fParticleCodes[3222]  = "\\Sigma^{+}";
    fParticleCodes[-3222] = "\\overline{\\Sigma^{+}}";
    fParticleCodes[1114]  = "\\Delta^{-}";
    fParticleCodes[-1114] = "\\overline{\\Delta^{-}}";
    fParticleCodes[3212]  = "\\Sigma^{0}";
    fParticleCodes[-3212] = "\\overline{\\Sigma^{0}}";
    fParticleCodes[2114]  = "\\Sigma^{0}";
    fParticleCodes[-2114] = "\\overline{\\Sigma^{0}}";

    fParticleCodes[fRootPDG] = fRootString;
}
