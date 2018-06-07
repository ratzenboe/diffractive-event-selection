/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: Sebastian Ratzenböck
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* CEPBGBase
 *
 * Code-base for analysis tasks looking at a subclass of events: 
 *  - events that pass a lhc16filter are evts that pass cuts that are also applied on real data
 *  - to filter these events a code base is constructed here with frequently used functions
 */

#include "TChain.h"
#include "TMath.h"
#include "TDatabasePDG.h"
#include "TCollection.h"
#include "AliAnalysisManager.h"
#include "AliEMCALHit.h"
#include "AliESDInputHandler.h"

#include "AliStack.h"
#include "AliESDtrack.h"
#include "AliESDtrackCuts.h"
#include "AliMultiplicitySelectionCP.h"
#include "AliMC.h"
#include "AliRunLoader.h"

#include "AliPID.h"

#include "EventDef.h"
#include "AliCEPBase.h"
/* #include "AliCEPUtils.h" */
#include "CEPBGBase.h"


#include <string>       // std::string
#include <cstddef>      // std::size_t
#include <algorithm>    // std::find

class CEPBGBase;    // your analysis class

ClassImp(CEPBGBase) // classimp: necessary for root

CEPBGBase::CEPBGBase() 
  : fESD(0)
  , fTrigger(0)
  , fTrackStatus(0)
  , fTracks(0)
  , fCEPUtil(0)
  , fPIDCombined(0)
  , fPIDResponse(0)
  , fAnalysisStatus(AliCEPBase::kBitConfigurationSet)
  , fTTmask(AliCEPBase::kTTBaseLine)
  , fTTpattern(AliCEPBase::kTTBaseLine) 
  , fHitsArray(0)
  , fHitFileName("EMCAL.Hits.root")
  , fHitFile(0)
  , fHitTree(0)
  , fHitBranch(0)
  , fHitDir(0)
  , fCurrentDir("")
  /* , fEvtStorge(0) */
{
    // default constructor, don't allocate memory here!
    // this is used by root for IO purposes, it needs to remain empty
}

//_____________________________________________________________________________
CEPBGBase::CEPBGBase(
  Long_t state,
  UInt_t TTmask, 
  UInt_t TTpattern,
  TString hitFileName) 
  : fESD(0)
  , fTrigger(0)
  , fTrackStatus(0)
  , fTracks(0)
  , fCEPUtil(0)
  , fPIDCombined(0)
  , fPIDResponse(0)
  , fAnalysisStatus(state)
  , fTTmask(TTmask)
  , fTTpattern(TTpattern)
  , fHitsArray(0)
  , fHitFileName(hitFileName)
  , fHitFile(0)
  , fHitTree(0)
  , fHitBranch(0)
  , fHitDir(0)
  , fCurrentDir("")
  /* , fEvtStorge(0) */
{
    // constructor
}

//_____________________________________________________________________________
CEPBGBase::~CEPBGBase()
{
    // destructor
    if (fTrigger) {
        delete fTrigger;
        fTrigger = 0x0;
    }
    if (fTrackStatus) {
        delete fTrackStatus;
        fTrackStatus = 0x0;
    }
    if (fTracks) {
        fTracks->SetOwner(kTRUE);
        fTracks->Clear();
        delete fTracks;
        fTracks = 0x0;
    }
    if(fPIDCombined) {
        delete fPIDCombined;
        fPIDCombined = 0x0;
    }
    if (fPIDResponse) {
        delete fPIDResponse;
        fPIDResponse = 0x0;
    }
    /* if (fEvtStorge) { */
    /*     delete fEvtStorge; */
    /*     fEvtStorge = 0x0; */ 
    /* } */
}

//_____________________________________________________________________________
void CEPBGBase::Initialize()
{
    fTrackStatus = new TArrayI();
    fTracks = new TObjArray();

    fTrigger = new AliTriggerAnalysis();
    fTrigger->SetDoFMD(kTRUE);
    fTrigger->SetFMDThreshold(0.3,0.5);
    fTrigger->ApplyPileupCuts(kTRUE);
    
    // AliCEPUtils
    fCEPUtil = new AliCEPUtils();
    fCEPUtil->SetTPCnclsS(3);             // limit for number of shared clusters
    fCEPUtil->SetTrackDCA(500);           // limit for DCA
    fCEPUtil->SetTrackDCAz(6);            // limit for DCAz
    fCEPUtil->SetTrackEtaRange(-0.9,0.9); // accepted eta range

    Bool_t isRun1 = fCEPUtil->checkstatus(fAnalysisStatus,
            AliCEPBase::kBitisRun1,AliCEPBase::kBitisRun1);
    fCEPUtil->InitTrackCuts(isRun1,1);  

    fPIDCombined = new AliPIDCombined();
    fPIDCombined->SetSelectedSpecies(AliPID::kSPECIES);  // default
    fPIDCombined->SetEnablePriors(kTRUE);
    fPIDCombined->SetDefaultTPCPriors();
    // use TPC and TOF for PID:
      // ... TPC and TOF
    UInt_t Maskin =
        AliPIDResponse::kDetITS |
        AliPIDResponse::kDetTPC |
        AliPIDResponse::kDetTOF;

    fPIDCombined->SetDetectorMask(Maskin);

    // event storage
    fEvtStorge = EventStorage();

    // emcal hits
    fHitsArray = new TClonesArray("AliEMCALHit",1000);
}

//_____________________________________________________________________________
void CEPBGBase::NewEvent(AliMCEvent* MCevt)
{
    EventDef ed;
    TParticle* part = 0x0;
    AliStack* stack = MCevt->Stack();
    Int_t nPrimaries = stack->GetNprimary();

    for (Int_t ii=4; ii<nPrimaries; ii++) {
        part = stack->Particle(ii);
        ed.AddTrack(ii, part->GetPdgCode(), part->GetMother(0), 
                    part->GetDaughter(0), part->GetDaughter(1), 
                    (part->GetStatusCode()==0) ? kFALSE : kTRUE);
    }
    // event storage specific actions
    ed.FinalizeEvent();
    fEvtStorge.AddEvent(ed);
    return ;
}

//_____________________________________________________________________________
Double_t CEPBGBase::GetMass(TObjArray* tracks, Int_t nTracksTT, 
                            TArrayI* TTindices, AliMCEvent* MCevt) const
{
    TParticle* part = 0x0;
    // construct invariant mass 
    TLorentzVector v_lor = TLorentzVector(0,0,0,0);
    // loop thru the particles of the event
    for (Int_t ii=0; ii<nTracksTT; ii++) 
    {
        // proper pointer into tracks and fTrackStatus
        Int_t trkIndex = TTindices->At(ii);
        // the original track
        AliESDtrack *tmptrk = (AliESDtrack*) tracks->At(trkIndex);
        // get MC truth
        Int_t MCind = tmptrk->GetLabel();
        // get the particle corresponding to MCind
        part = GetPartByLabel(MCind, MCevt);
        if (!part) return -1.;
        // set MC mass and momentum
        TLorentzVector lv;
        part->Momentum(lv);
        v_lor += lv;        
    }
    return v_lor.M();
}

//_____________________________________________________________________________
std::vector<Double_t> CEPBGBase::GetMassPermute(TObjArray* tracks, Int_t nTracksTT, 
                                                TArrayI* TTindices, AliMCEvent* MCevt) const
{
    TParticle *part_1(0x0), *part_2(0x0);
    Double_t charge_1, charge_2;
    TLorentzVector v_lor_1, v_lor_2;
    // output vector
    std::vector<Double_t> mass_vec;
    mass_vec.clear();

    // construct invariant mass:
    //      -> combine each possible combination of pi+pi-
    for (Int_t ii(0); ii<nTracksTT-1; ii++) 
    {
        Int_t trkIndex = TTindices->At(ii);
        AliESDtrack *tmptrk = (AliESDtrack*) tracks->At(trkIndex);
        Int_t MCind = tmptrk->GetLabel();
        part_1 = GetPartByLabel(MCind, MCevt);
        if (!part_1) continue;
        // set lorentz vector 1
        part_1->Momentum(v_lor_1);
        charge_1 = TDatabasePDG::Instance()->GetParticle(part_1->GetPdgCode())->Charge();

        // combine the masses of two pions to 
        for (Int_t kk=ii+1; kk<nTracksTT; kk++) 
        {
            Int_t trkIndex = TTindices->At(kk);
            AliESDtrack *tmptrk = (AliESDtrack*) tracks->At(trkIndex);
            Int_t MCind = tmptrk->GetLabel();
            part_2 = GetPartByLabel(MCind, MCevt);
            if (!part_2) continue;
            charge_2 = TDatabasePDG::Instance()->GetParticle(part_2->GetPdgCode())->Charge();
            if (charge_2 == charge_1) continue;
            // set lorentz vector 2
            part_2->Momentum(v_lor_2);
            mass_vec.push_back((v_lor_1+v_lor_2).M());
        }
    }

    return mass_vec;
}

//_____________________________________________________________________________
TParticle* CEPBGBase::GetPartByLabel(Int_t MCind, AliMCEvent* MCevt) const
{
    TParticle* part = 0x0;
    AliStack* stack = MCevt->Stack();
    Int_t nPrimaries = stack->GetNprimary();

    if (MCind <= 0) {
        printf("<W> MC index below 0\nCorrection to absolute value!\n"); 
        if (abs(MCind) <= nPrimaries) MCind = abs(MCind);
        else { printf("\n<E> No MC-particle info available!\n\n"); return 0x0; }
    }

    if (MCevt) part = stack->Particle(MCind);
    else { printf("\n<E> No MC-particle info available!\n\n"); return 0x0; }

    return part;
}

//_____________________________________________________________________________
TLorentzVector CEPBGBase::GetXLorentzVector(AliMCEvent* MCevent) const
{
    AliStack* stack = MCevent->Stack();
    Int_t nPrimaries = stack->GetNprimary();

    TLorentzVector lvtmp;
    TLorentzVector lvprod = TLorentzVector(0,0,0,0);

    stack->Particle(4)->Momentum(lvtmp);
    lvprod  = lvtmp;
    for (Int_t ii=5; ii<nPrimaries; ii++) {
        if (stack->Particle(ii)->GetMother(0)==0) {
            stack->Particle(ii)->Momentum(lvtmp);
            lvprod += lvtmp;
        }
    }
    return lvprod;
}

//_____________________________________________________________________________
Bool_t CEPBGBase::EvtFullRecon(TObjArray* tracks, Int_t nTracksTT, 
                               TArrayI* TTindices, AliMCEvent* mcEvt) const
{
    // get lorentzvector of the X particle
    TLorentzVector X_lor = GetXLorentzVector(mcEvt);
    TParticle* part = 0x0;
    
    // calculate the lorentzvector of the measured particles and check if they agree with X_lor
    TLorentzVector measured_lor = TLorentzVector(0,0,0,0);
    for (Int_t ii=0; ii<nTracksTT; ii++) {
        // proper pointer into tracks and fTrackStatus
        Int_t trkIndex = TTindices->At(ii);
        // the original track
        AliESDtrack *tmptrk = (AliESDtrack*) tracks->At(trkIndex);
        // get MC truth
        Int_t MCind = tmptrk->GetLabel();
        part = GetPartByLabel(MCind, mcEvt);
        if (!part) return kFALSE;
        // set lorentz vector 
        TLorentzVector lv;
        part->Momentum(lv);
        // total lorentz-vector
        measured_lor += lv;        
    }
    Double_t m_diff = measured_lor.M() - X_lor.M();
    if (m_diff < 0) m_diff = -m_diff;
    if (m_diff < 1e-5) return kTRUE;
    else return kFALSE;
}

//_____________________________________________________________________________
Bool_t CEPBGBase::MatchTracks(AliESDCaloCluster* clust, TObjArray* tracks, Int_t nTracksTT, 
                              TArrayI* TTindices, AliMCEvent* MCevt, Double_t& dPhiEtaMin) 
{
    dPhiEtaMin = 999.;
     
    /* printf("Distance to track in phi: %-6.2f, in eta: %-6.2f\n", */ 
    /*         clust->GetTrackDx(), clust->GetTrackDz()); */
    Float_t x[3];
    clust->GetPosition(x);
    TVector3 v3(x[0], x[1], x[2]);
    // v3 phi is in the range [-pi,pi) -> map it to [0, 2pi)
    Double_t cluster_phi = (v3.Phi()>0.) ? v3.Phi() : v3.Phi() + 2.*TMath::Pi();
    Double_t cluster_eta = v3.Eta();
    /* printf("EMCal cluster positions: phi: %-6.2f, eta: %-6.2f\n", cluster_phi, cluster_eta); */

    for (Int_t kk(0); kk<nTracksTT; kk++) {
        // proper pointer into tracks and fTrackStatus
        Int_t trkIndex = TTindices->At(kk);
        // the original track
        AliESDtrack *tmptrk = (AliESDtrack*) tracks->At(trkIndex);
        TParticle* part = GetPartByLabel(tmptrk->GetLabel(), MCevt);
        if (!part) continue;
        // set MC mass and momentum
        TLorentzVector lv;
        part->Momentum(lv);
        // track position on emcal
        Double_t trkPhiOnEmc = tmptrk->GetTrackPhiOnEMCal();
        // Map phi to [0,2pi)
        trkPhiOnEmc = (trkPhiOnEmc>0.) ? trkPhiOnEmc : trkPhiOnEmc+2.*TMath::Pi();
        if (trkPhiOnEmc<0.) trkPhiOnEmc=-999.;
        Double_t trkEtaOnEmc = tmptrk->GetTrackEtaOnEMCal();
        // no matching if at least one has value -999.
        if (trkPhiOnEmc==-999. || trkEtaOnEmc==-999.) continue;
        // in case of track on emcal: calculate distance in phi and eta
        Double_t dEta = trkEtaOnEmc - cluster_eta;
        Double_t dPhi = trkPhiOnEmc - cluster_phi;
        Double_t dPhiEta = TMath::Sqrt( dEta*dEta + dPhi*dPhi );

        dPhiEtaMin = (dPhiEtaMin<dPhiEta) ? dPhiEtaMin : dPhiEta;
    }
    return (dPhiEtaMin==999.) ? kFALSE : kTRUE;
}

//_____________________________________________________________________________
Bool_t CEPBGBase::IsClusterFromPDG(AliESDCaloCluster* clust, Int_t& pdg, AliMCEvent* MCevt)
{
    AliStack* stack = MCevt->Stack();
    Int_t MCClusterIndex = clust->GetLabel();
    Int_t particle_pdg = abs(stack->Particle(MCClusterIndex)->GetPdgCode());
    // we check if the cluster is made by a scondary originating from a primary particle
    Int_t mc_idx = MCClusterIndex;
    while (!stack->IsPhysicalPrimary(mc_idx)) {
        // check if mother particle is primary
        mc_idx = stack->Particle(mc_idx)->GetMother(0);
        particle_pdg = abs(stack->Particle(mc_idx)->GetPdgCode());
    }
    Bool_t isclusterfrompart = (particle_pdg==pdg);
    pdg = particle_pdg;
    return isclusterfrompart;
}

//_____________________________________________________________________________
TString CEPBGBase::GetDirFromFullPath(const char* fullpath)
{
    std::string curr_dir(fullpath);
    // find last occurance of "/" (determining the full path to the directory)
    std::size_t path_end_idx = curr_dir.find_last_of("/\\");
    // take only the substring to the last occurance (+1 to also get the slash /)
    TString outstr((curr_dir.substr(0,path_end_idx+1)).c_str());

    return outstr;
}

//_____________________________________________________________________________
Bool_t CEPBGBase::UpdateGlobalVars(const char* currentFileName, Int_t entry)
{
    // CurrentFileName: func from AliAnalysisTaskSE
    if (fCurrentDir=="" || fCurrentDir != GetDirFromFullPath(currentFileName)) {
        fCurrentDir = GetDirFromFullPath(currentFileName);
        fHitFile = TFile::Open(fCurrentDir+fHitFileName);
    }
    if (!fHitFile) return kFALSE;
    TString iev_str;
    iev_str.Form("Event%i", entry);
    fHitDir = fHitFile->GetDirectory(iev_str);
    if (!fHitDir) return kFALSE;
    // get the hit-tree corresponding to the event 
    fHitDir->GetObject("TreeH", fHitTree);
    if (!fHitTree) return kFALSE;
    fHitBranch = fHitTree->GetBranch("EMCAL");
    fHitBranch->SetAddress(&fHitsArray);
    if (!fHitBranch) return kFALSE;

    return kTRUE;
}

//_____________________________________________________________________________
Bool_t CEPBGBase::IsPionEvt(TObjArray* tracks, 
                            Int_t nTracksTT, 
                            TArrayI* TTindices, 
                            AliPIDResponse* pidResponse,
                            AliPIDCombined* pidCombined,
                            AliMCEvent* MCevt) const
{
    Bool_t isPionEvt = kTRUE;
    TParticle* part = 0x0;
    Double_t stat, probs[AliPID::kSPECIES];
    for (Int_t ii=0; ii<nTracksTT; ii++) 
    {
        Int_t trkIndex = TTindices->At(ii);
        AliESDtrack *tmptrk = (AliESDtrack*) tracks->At(trkIndex);
        // bayes pid
        stat = pidCombined->ComputeProbabilities(tmptrk, pidResponse, probs);
        isPionEvt = isPionEvt && (probs[AliPID::kPion]>=0.9);

        // print option to check validity of result
        /* if (MCevt){ */
        /*     part = GetPartByLabel(tmptrk->GetLabel(), MCevt); */
        /*     if (!part) continue; */
        /* } */
    }
    return isPionEvt;
}


//_____________________________________________________________________________
Bool_t CEPBGBase::lhc16filter(AliESDEvent* esd_evt, Int_t nTracksAccept, 
                              UInt_t TTmask, UInt_t TTpattern,
                              Int_t& nTracksTT, TArrayI*& TTindices) const
{
    // - nTracksAccept
    nTracksTT = fCEPUtil->countstatus(fTrackStatus, TTmask, TTpattern, TTindices);
    if (nTracksTT!=nTracksAccept) return kFALSE;
    
    return EventFilter(esd_evt, nTracksAccept);
}

//_____________________________________________________________________________
// Overwritten to account for 3+ background evts
Bool_t CEPBGBase::lhc16filter(AliESDEvent* esd_evt, std::vector<Int_t> nTrksAcc_vec, 
                              UInt_t TTmask, UInt_t TTpattern,
                              Int_t& nTracksTT, TArrayI*& TTindices) const
{
    // there are multiple a numbers of accepted N_tracks
    nTracksTT = fCEPUtil->countstatus(fTrackStatus, TTmask, TTpattern, TTindices);
    if (std::find(nTrksAcc_vec.begin(), nTrksAcc_vec.end(), nTracksTT) == nTrksAcc_vec.end()) {
        return kFALSE; 
    }
    return EventFilter(esd_evt, nTracksTT);
}

//_____________________________________________________________________________
Bool_t CEPBGBase::EventFilter(AliESDEvent* esd_evt, Int_t nTracksAccept) const
{
    // here we filter for events that satisfy the condition (filter-bit 107 in CEP evts)
    // 0: remove pileup
    // 1: CCUP13
    // 3: !V0
    // 5: !AD
    // 6: *FO>=1 (to replay OSMB) && *FO<=trks
    //
    // remove pileup
    if (esd_evt->IsPileupFromSPD(3,0.8,3.,2.,5.)) return kFALSE;
    // CCUP13
    Bool_t isV0A = fTrigger->IsOfflineTriggerFired(esd_evt, AliTriggerAnalysis::kV0A);
    Bool_t isV0C = fTrigger->IsOfflineTriggerFired(esd_evt, AliTriggerAnalysis::kV0C);
    UInt_t isSTGTriggerFired;
    const AliMultiplicity *mult = (AliMultiplicity*)esd_evt->GetMultiplicity();
    TBits foMap = mult->GetFastOrFiredChips();
    isSTGTriggerFired = IsSTGFired(&foMap,0) ? (1<<0) : 0;
    for (Int_t ii=1; ii<=10; ii++) {
        isSTGTriggerFired |= IsSTGFired(&foMap,ii) ? (1<<ii) : 0;
    }
    if (isV0A || isV0C || !isSTGTriggerFired) return kFALSE;
    // !V0
    // is encorporated in CCUP13
    // !AD
    Bool_t isADA = fTrigger->IsOfflineTriggerFired(esd_evt, AliTriggerAnalysis::kADA);
    Bool_t isADC = fTrigger->IsOfflineTriggerFired(esd_evt, AliTriggerAnalysis::kADC);
    if (isADA || isADC) return kFALSE;
    // *FO>=1 (to replay OSMB) && *FO<=trks
    Short_t nFiredChips[4] = {0};
    nFiredChips[0] = mult->GetNumberOfFiredChips(0);
    nFiredChips[1] = mult->GetNumberOfFiredChips(1);
    for (Int_t ii=0;    ii<400; ii++) nFiredChips[2] += foMap[ii]>0 ? 1 : 0;
    for (Int_t ii=400; ii<1200; ii++) nFiredChips[3] += foMap[ii]>0 ? 1 : 0;
    Bool_t firedChipsOK = kTRUE;
    for (Int_t ii=0; ii<4; ii++) {
      firedChipsOK =
        firedChipsOK &&
        (nFiredChips[ii]>=1) &&
        (nFiredChips[ii]<=nTracksAccept);
    }
    if (!firedChipsOK) return kFALSE;

    // if the cuts are passed the filter returns true
    return kTRUE;
}

//_____________________________________________________________________________
//
//
//------------------------------------------------------------------------------
// code to check if the STG trigger had fired
// code from Evgeny Kryshen
// dphiMin/dphiMax specifies the range for the angle between two tracks
Bool_t CEPBGBase::IsSTGFired(TBits* fFOmap,Int_t dphiMin,Int_t dphiMax) const
{

  Int_t hitcnt = 0;
  Bool_t stg = kFALSE;
  
  if (!fFOmap) {
    // printf("<AliAnalysisTaskCEP::IsSTGFired> Problem with F0map - a!\n");
    return stg;
  }

  Int_t n1 = fFOmap->CountBits(400);
  Int_t n0 = fFOmap->CountBits()-n1;
  if (n0<1 || n1<1) {
    // printf("<AliAnalysisTaskCEP::IsSTGFired> Problem with F0map - b!\n");
    return stg;
  }
  
  Bool_t l0[20]={0};
  Bool_t l1[40]={0};
  Bool_t phi[20]={0};
  for (Int_t i=0;   i< 400; ++i) if (fFOmap->TestBitNumber(i)) l0[      i/20] = 1;
  for (Int_t i=400; i<1200; ++i) if (fFOmap->TestBitNumber(i)) l1[(i-400)/20] = 1;
  for (Int_t i=0; i<20; ++i) phi[i] = l0[i] & (l1[(2*i)%40] | l1[(2*i+1)%40] | l1[(2*i+2)%40] | l1[(2*i+39)%40]);
  for (Int_t dphi=dphiMin;dphi<=dphiMax;dphi++) {
    for (Int_t i=0; i<20; ++i) {
      stg |= phi[i] & phi[(i+dphi)%20];
      if (phi[i] & phi[(i+dphi)%20]) hitcnt++;
    }
  }
  // printf("hitcnt: %i\n",hitcnt);

  return stg;
}

///////////////////////////////////////////////////////////////////////////////
// --------------------------- Print functions --------------------------------
//_____________________________________________________________________________
void CEPBGBase::PrintTracks(TObjArray* tracks, Int_t nTracksTT, 
                            TArrayI* TTindices) const
{
    printf("------------ Tracks --------------------\n");
    for (Int_t ii=0; ii<nTracksTT; ii++) 
    {
        // proper pointer into tracks and fTrackStatus
        Int_t trkIndex = TTindices->At(ii);
        // the original track
        AliESDtrack *tmptrk = (AliESDtrack*) tracks->At(trkIndex);
        printf("Track label: %i", tmptrk->GetLabel());
        printf("  Track E: %-6.8f", tmptrk->E());
        printf("\n");
    }
    printf("------------ Tracks end ----------------\n");

    return ;
}

//_____________________________________________________________________________
void CEPBGBase::PrintStack(AliMCEvent* MCevent, Bool_t prim) const
{
    AliStack* stack = MCevent->Stack();
    Int_t nPrimaries = stack->GetNprimary();
    Int_t nTracks = stack->GetNtrack();

    /* TLorentzVector lvtmp; */
    printf("\n-----------------------------------------------\n");
    Int_t nParticles = prim ? nPrimaries : nTracks;
    for (Int_t ii(4); ii<nParticles; ii++) {
        TParticle* part = stack->Particle(ii);
        printf("%i: %-13s: E=%-6.8f, Mother: %i, Daugther 0: %i, 1: %i", 
                ii, part->GetName(), part->Energy(), 
                part->GetMother(0), part->GetDaughter(0), part->GetDaughter(1));
        if (part->GetStatusCode()==1) printf("   final");
        printf("\n");
        if (ii==nPrimaries-1) printf("-------------- Primaries end ------------------\n");
    }
    printf("-----------------------------------------------\n");

    return ;
}

//_____________________________________________________________________________
void CEPBGBase::PrintEMCALHits() const
{
    // a single particle may produce many hits in the emcal -> record particles 
    // and if we see a duplicate we ignore it
    std::vector<Int_t> parent_v;
    for (Int_t iHit(0); iHit<fHitBranch->GetEntries(); iHit++){
        fHitBranch->GetEntry(iHit);    
        TIter next(fHitsArray);
        AliEMCALHit* hit;
        while( (hit=dynamic_cast<AliEMCALHit*>(next())) )
        {
            // if the parent particle has already been counted we continue
            if(std::find(parent_v.begin(), parent_v.end(), hit->GetIparent()) 
                    != parent_v.end()) continue;

            printf("Initial energy of parent part: %-3.8f", hit->GetIenergy());
            printf("  <- Parent Id: %i", hit->GetIparent());
            printf("  <- GetPrimary(): %i", hit->GetPrimary());
            printf("  <- Primary entrance E(): %-3.8f", hit->GetPe());
            printf("  <- E depon: %-3.8f", hit->GetEnergy());
            printf("  <- GetId(): %i", hit->GetId());
            printf("\n");

            parent_v.push_back(hit->GetIparent());

            /* if (isSignal) fEMCalSecondaryE_SIG->Fill(hit->GetIenergy()); */
            /* else fEMCalSecondaryE_BG->Fill(hit->GetIenergy()); */
        }
    }
    fHitsArray->Clear();
}

