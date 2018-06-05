/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
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

/* AliAnaysisTaskMCInfo
 *
 * empty task which can serve as a starting point for building an analysis
 * as an example, one histogram is filled
 */

#include "TChain.h"
#include "TMath.h"
#include "TDatabasePDG.h"
#include "TCollection.h"
#include "AliAnalysisTask.h"
#include "AliAnalysisManager.h"
#include "AliEMCALHit.h"
#include "AliESDInputHandler.h"

#include "AliStack.h"
#include "AliESDtrack.h"
#include "AliESDtrackCuts.h"
#include "AliMultiplicitySelectionCP.h"
#include "AliMC.h"
#include "AliRunLoader.h"

#include "AliCEPBase.h"
/* #include "AliCEPUtils.h" */
#include "AliAnalysisTaskBG3plus.h"

#include <string>       // std::string
#include <cstddef>      // std::size_t
#include <algorithm>    // std::find

class AliAnalysisTaskBG3plus;    // your analysis class

using namespace std;            // std namespace: so you can do things like 'cout'

ClassImp(AliAnalysisTaskBG3plus) // classimp: necessary for root

AliAnalysisTaskBG3plus::AliAnalysisTaskBG3plus() 
  : AliAnalysisTaskSE()
  , fESD(0)
  , fTrigger(0)
  , fTrackStatus(0)
  , fTracks(0)
  , fCEPUtil(0)
  , fAnalysisStatus(AliCEPBase::kBitConfigurationSet)
  , fTTmask(AliCEPBase::kTTBaseLine)
  , fTTpattern(AliCEPBase::kTTBaseLine) 
  , fOutList(0)
  , fInvMass_3trks(0)
{
    // default constructor, don't allocate memory here!
    // this is used by root for IO purposes, it needs to remain empty
}

//_____________________________________________________________________________
AliAnalysisTaskBG3plus::AliAnalysisTaskBG3plus(const char* name,
  Long_t state,
  UInt_t TTmask, 
  UInt_t TTpattern,
  TString hitFileName) 
  : AliAnalysisTaskSE(name)
  , fESD(0)
  , fTrigger(0)
  , fTrackStatus(0)
  , fTracks(0)
  , fCEPUtil(0)
  , fAnalysisStatus(state)
  , fTTmask(TTmask)
  , fTTpattern(TTpattern)
  , fOutList(0)
  , fInvMass_3trks(0)
{
    // constructor
    /* DefineInput(0, TChain::Class());    // define the input of the analysis: */ 
    /*                                     // in this case we take a 'chain' of events */
    /*                                     // this chain is created by the analysis manager, */ 
    /*                                     // so no need to worry about it, */ 
    /*                                     // it does its work automatically */
                                        
    DefineOutput(1, TList::Class());    // define the ouptut of the analysis: 
                                        // in this case it's a list of histograms 
}

//_____________________________________________________________________________
AliAnalysisTaskBG3plus::~AliAnalysisTaskBG3plus()
{
    // destructor
    if(fOutList) {
        delete fOutList;     // at the end of your task, 
                             // it is deleted from memory by calling this function
    }
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
}

//_____________________________________________________________________________
void AliAnalysisTaskBG3plus::UserCreateOutputObjects()
{
    // create output objects
    //
    // this function is called ONCE at the start of the analysis (RUNTIME)
    // here the histograms and other objects are created

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

   
    // the histograms are added to a tlist which is in the end saved to an output file
    fOutList = new TList();             // this is a list which will contain all of your histograms
    fOutList->SetOwner(kTRUE);          // memory stuff: the list is owner of all objects 
                                        // it contains and will delete them if requested 
    fInvMass_3trks = new TH1F("fInvMass_3trks", "fInvMass_3trks",100, 0, 3);
  
    fOutList->Add(fInvMass_3trks);          

    PostData(1, fOutList);              // postdata will notify the analysis manager of changes 
                                        // and updates to the fOutList object. 
                                        // the manager will in the end take care of writing 
                                        // the output to file so it needs to know what's 
                                        // in the output
}
//_____________________________________________________________________________
void AliAnalysisTaskBG3plus::UserExec(Option_t *)
{
    // user exec
    // this function is called once for each event
    // the manager will take care of reading the events from file, 
    // and with the static function InputEvent() you have access to the current event. 
    // once you return from the UserExec function, 
    // the manager will retrieve the next event from the chain
    fESD = dynamic_cast<AliESDEvent*>(InputEvent()); 
    if(!fESD) return;
    // ///////////////////////////////////////////////////////////////////////////////
    // get MC event (fMCEvent is member variable from AliAnalysisTaskSE)
    fMCEvent = MCEvent();
    if (fMCEvent) { if (fMCEvent->Stack()==NULL) fMCEvent=NULL; }
    AliStack *stack = NULL;
    if (fMCEvent) stack = fMCEvent->Stack();
    else { printf("<E> No MC-event available!\n"); return ; }
    // ///////////////////////////////////////////////////////////////////////////////

    // ///////////////////////////////////////////////////////////////////////////////
    // ------------------------- event filter ----------------------------------------
    Int_t nTracks = fCEPUtil->AnalyzeTracks(fESD, fTracks, fTrackStatus);
    Int_t nTracksTT;
    std::vector<Int_t> nTracksAccept{ 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    TArrayI *TTindices  = new TArrayI();
    Bool_t isGoodEvt = lhc16filter(fESD, nTracksAccept, nTracksTT, TTindices);
    if (!isGoodEvt) return ;
    // here we have now events which passed the track selection
    // ///////////////////////////////////////////////////////////////////////////////
    if (nTracksTT==2){
        // do not consider full recon evts in this class
        if (EvtFullRecon(fTracks, nTracksTT, TTindices, fMCEvent)) return ;
        // here we only have 2-track fd-evts
        fInvMass_FD->Fill(GetMass(fTracks, nTracksTT, TTindices, fMCEvent));
        // event is finished
    } else {
        // otherwise the event has more than 2 particles and the BG can be simulated by 3+ tracks
        if (nTracksTT==3) {
            std::vector<Double_t> mass_vec = GetMassPermute(fTracks,nTracksTT,TTindices,fMCEvent);
            for (Double_t mass : mass_vec) fInvMass_3trks->Fill(mass);
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // ---------------------- Print the event stack ----------------------------------
    /* PrintStack(fMCEvent, kFALSE); */
    // ---------------------- Print the AliESDtracks ---------------------------------
    /* PrintTracks(fESD); */
    //////////////////////////////////////////////////////////////////////////////////
    
    PostData(1, fOutList);          // stream the results the analysis of this event to
                                    // the output manager which will take care of writing
                                    // it to a file
}
//_____________________________________________________________________________
void AliAnalysisTaskBG3plus::Terminate(Option_t *)
{
    // terminate
    // called at the END of the analysis (when all events are processed)
}

//_____________________________________________________________________________
Double_t AliAnalysisTaskBG3plus::GetMass(TObjArray* tracks, Int_t nTracksTT, 
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
        // set MC mass and momentum
        part->Momentum(lv);
        v_lor += lv;        
    }

    return v_lor.M();
}

//_____________________________________________________________________________
std::vector<Double_t> AliAnalysisTaskBG3plus::GetMassPermute(TObjArray* tracks, Int_t nTracksTT, 
                                         TArrayI* TTindices, AliMCEvent* MCevt) const
{
    TParticle* part_1(0x0), part_2(0x0);
    Double_t charge_1, charge_2;
    TLorentzVector v_lor_1, v_lor_2;
    // output vector
    std::vector<Double_t> mass_vec;
    mass_vec.clear();

    // construct invariant mass:
    //      -> combine each possible combination of pi+pi-
    for (Int_t ii(0); ii<TTindices-1; ii++) 
    {
        Int_t trkIndex = TTindices->At(ii);
        AliESDtrack *tmptrk = (AliESDtrack*) tracks->At(trkIndex);
        Int_t MCind = tmptrk->GetLabel();
        part_1 = GetPartByLabel(MCind, MCevt);
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
TParticle* AliAnalysisTaskBG3plus::GetPartByLabel(Int_t MCind, AliMCEvent* MCevt) const
{
    TParticle* part = 0x0;
    AliStack* stack = MCevent->Stack();
    Int_t nPrimaries = stack->GetNprimary();

    if (MCind <= 0) {
        printf("<W> MC index below 0\nCorrection to absolute value!\n"); 
        if (abs(MCind) <= nPrimaries) MCind = abs(MCind);
        else return { printf("\n<E> No MC-particle info available!\n\n"); gSystem->Exit(1); }
    }

    if (MCevt) part = stack->Particle(MCind);
    else { printf("\n<E> No MC-particle info available!\n\n"); gSystem->Exit(1); }

    return part;
}

//_____________________________________________________________________________
TLorentzVector AliAnalysisTaskBG3plus::GetXLorentzVector(AliMCEvent* MCevent) const
{
    AliStack* stack = MCevent->Stack();
    Int_t nPrimaries = stack->GetNprimary();
    // MC generator and process type
    /* TString MCGenerator; */
    /* Int_t MCProcess; */ 
    /* fCEPUtil->DetermineMCprocessType(MCEvent,MCGenerator,MCProcess); */

    TLorentzVector lvtmp;
    TLorentzVector lvprod = TLorentzVector(0,0,0,0);
    /* if ( MCGenerator.EqualTo("Pythia") && MCProcess==106 ) */
    /* { */
    stack->Particle(4)->Momentum(lvtmp);
    lvprod  = lvtmp;
    for (Int_t ii=5; ii<nPrimaries; ii++) {
        if (stack->Particle(ii)->GetMother(0)==0) {
            stack->Particle(ii)->Momentum(lvtmp);
            lvprod += lvtmp;
        }
    }
    /* } else { printf("<E> MC-generator not pythia CEP!"); gSystem->Exit(1); } */
    return lvprod;
}

//_____________________________________________________________________________
Bool_t AliAnalysisTaskBG3plus::EvtFullRecon(TObjArray* tracks, Int_t nTracksTT, 
                                            TArrayI* TTindices, AliMCEvent* mcEvt) const
{
    AliStack* stack = mcEvt->Stack();
    Int_t nTracksPrimMC = stack->GetNprimary();
    
    // get lorentzvector of the X particle
    TLorentzVector X_lor = GetXLorentzVector(mcEvt);
    
    // calculate the lorentzvector of the measured particles and check if they agree with X_lor
    TLorentzVector measured_lor = TLorentzVector(0,0,0,0);
    for (Int_t ii=0; ii<nTracksTT; ii++) {
        // proper pointer into tracks and fTrackStatus
        Int_t trkIndex = TTindices->At(ii);
        // the original track
        AliESDtrack *tmptrk = (AliESDtrack*) tracks->At(trkIndex);
        // get MC truth
        Int_t MCind = tmptrk->GetLabel();
        if (MCind <= 0) {
            printf("<W> MC index below 0\nCorrection to absolute value!\n"); 
            if (abs(MCind) <= nTracksPrimMC) MCind = abs(MCind);
            else return ;
        }
        if (fMCEvent) {
            TParticle* part = stack->Particle(MCind);
            // set MC mass and momentum
            TLorentzVector lv;
            part->Momentum(lv);
            measured_lor += lv;        
        } else { printf("\n<E> No MC-particle info available!\n\n"); gSystem->Exit(1); }
    }
    Double_t m_diff = measured_lor.M() - X_lor.M();
    if (m_diff < 0) m_diff = -m_diff;
    Bool_t isFullRecon(kFALSE);
    if (m_diff < 1e-5) return kTRUE;
    else return kFALSE;
}
//_____________________________________________________________________________
Bool_t AliAnalysisTaskBG3plus::lhc16filter(AliESDEvent* esd_evt, Int_t nTracksAccept, 
        Int_t& nTracksTT, TArrayI*& TTindices)
{
    // - nTracksAccept
    nTracksTT = fCEPUtil->countstatus(fTrackStatus, fTTmask, fTTpattern, TTindices);
    if (nTracksTT!=nTracksAccept) return kFALSE;
    
    return EventFilter(esd_evt, nTracksAccept);
}

//_____________________________________________________________________________
// Overwritten to account for 3+ background evts
Bool_t AliAnalysisTaskBG3plus::lhc16filter(AliESDEvent* esd_evt, std::vector<Int_t> nTrksAcc_vec, 
        Int_t& nTracksTT, TArrayI*& TTindices)
{
    // there are multiple a numbers of accepted N_tracks
    nTracksTT = fCEPUtil->countstatus(fTrackStatus, fTTmask, fTTpattern, TTindices);
    if (std::find(nTrksAcc_vec.begin(), nTrksAcc_vec.end(), nTracksTT) == nTrksAcc_vec.end()) {
        return kFALSE; 
    }
    return EventFilter(esd_evt, nTracksTT);
}

//_____________________________________________________________________________
Bool_t AliAnalysisTaskBG3plus::EventFilter(AliESDEvent* esd_evt, Int_t nTracksAccept)
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
    isSTGTriggerFired  = IsSTGFired(&foMap,0) ? (1<<0) : 0;
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
Bool_t AliAnalysisTaskBG3plus::IsSTGFired(TBits* fFOmap,Int_t dphiMin,Int_t dphiMax)
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
void AliAnalysisTaskBG3plus::PrintTracks(AliESDEvent* esd_evt) const
{
    printf("------------ Tracks --------------------\n");
    for (Int_t ii(0); ii<esd_evt->GetNumberOfTracks(); ii++){
        AliESDtrack* trk = esd_evt->GetTrack(ii);
        printf("Track label: %i", trk->GetLabel());
        printf("  Track E: %-6.8f", trk->E());
        printf("\n");
    }
    printf("------------ Tracks end ----------------\n");

    return ;
}

//_____________________________________________________________________________
void AliAnalysisTaskBG3plus::PrintStack(AliMCEvent* MCevent, Bool_t prim) const
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
        if (fGeometry->IsInEMCALOrDCAL(part->Vx(), part->Vy(), part->Vz())) 
            printf("    VERTEX IN EMCAL"); 
        printf("\n");
        if (ii==nPrimaries-1) printf("-------------- Primaries end ------------------\n");
        /* if (stack->Particle(ii)->GetMother(0)==0) { */
    }
    printf("-----------------------------------------------\n");
    AliESDCaloCells* emcal_cells = fESD->GetEMCALCells();
    AliESDCaloCells* phos_cells  = fESD->GetPHOSCells();

    if (emcal_cells->GetNumberOfCells()>0){ 
        printf("MC labels of particles in emcal cells:\n");
        for (Int_t kk(0); kk<emcal_cells->GetNumberOfCells(); kk++)
            printf("%i: MC label: %i,  E:%-6.2f, Time: %-.3e\n", 
                    kk, 
                    emcal_cells->GetMCLabel(kk), 
                    emcal_cells->GetAmplitude(kk), 
                    emcal_cells->GetTime(kk));
        printf("Emcal clusters: %i\n", fESD->GetNumberOfCaloClusters());
        if (fESD->GetNumberOfCaloClusters()>0) {
            for (Int_t kk(0); kk<fESD->GetNumberOfCaloClusters(); kk++){
                if (fESD->GetCaloCluster(kk)->IsPHOS()) continue;
                printf("%i: MC label: %i, E:%-6.2f, Dx:%-6.2f, Dphi:%-6.2f\n", 
                        kk,
                        fESD->GetCaloCluster(kk)->GetLabel(),
                        fESD->GetCaloCluster(kk)->E(),
                        fESD->GetCaloCluster(kk)->GetTrackDx(),
                        fESD->GetCaloCluster(kk)->GetTrackDz());
            }
        }
        printf("-----------------------------------------------\n");
    }

    if (phos_cells->GetNumberOfCells()>0){ 
        printf("MC labels of particles in PHOS:\n");
        for (Int_t kk(0); kk<phos_cells->GetNumberOfCells(); kk++)
            printf("%i: MC label: %i,  E:%-6.2f, Time: %-.3e\n", 
                    kk, 
                    phos_cells->GetMCLabel(kk), 
                    phos_cells->GetAmplitude(kk), 
                    phos_cells->GetTime(kk));
        printf("-----------------------------------------------\n");
    }

    return ;
}


