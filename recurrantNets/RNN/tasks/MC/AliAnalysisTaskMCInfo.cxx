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
#include "TH1F.h"
#include "TMath.h"
#include "TList.h"
#include "TDatabasePDG.h"
#include "AliAnalysisTask.h"
#include "AliTriggerAnalysis.h"
#include "AliAnalysisManager.h"
#include "AliESDEvent.h"
#include "AliESDInputHandler.h"

#include "AliMCEvent.h"
#include "AliStack.h"
#include "AliESDtrack.h"
#include "AliESDtrackCuts.h"
#include "AliMultiplicitySelectionCP.h"

#include "AliCEPBase.h"
#include "AliCEPUtils.h"
#include "AliAnalysisTaskMCInfo.h"

class AliAnalysisTaskMCInfo;    // your analysis class

using namespace std;            // std namespace: so you can do things like 'cout'

ClassImp(AliAnalysisTaskMCInfo) // classimp: necessary for root

AliAnalysisTaskMCInfo::AliAnalysisTaskMCInfo() 
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
  , fGammaE(0)
  , fNeutralPDG(0)
  , fEmcalHitMothers_SIG(0)
  , fEmcalHitMothers_BG(0)
  , fDistBadChannel_SIG(0)
  , fDistBadChannel_BG(0)
  , fEMCnClus_SIG(0)
  , fEMCnClus_BG(0)
  , fEMCnMatchedClus_SIG(0)
  , fEMCnMatchedClus_BG(0)
  , fEMC_nClusVSnMatched_SIG(0)
  , fEMC_nClusVSnMatched_BG(0)
  , fPHOSnClus_SIG(0)
  , fPHOSnClus_BG(0)
  , fPHOSenergy_SIG(0)
  , fPHOSenergy_BG(0)
  , fEMCenergy_SIG(0)
  , fEMCenergy_BG(0)
  , fEMC_nClusVSenergy_SIG(0)
  , fEMC_nClusVSenergy_BG(0)
  , fPHOS_nClusVSenergy_SIG(0)
  , fPHOS_nClusVSenergy_BG(0)
  , fEMCDphi_SIG(0)
  , fEMCDphi_BG(0)
  , fEMCDz_SIG(0)
  , fEMCDz_BG(0)
  , fEMCal_dphiEta_SIG(0)
  , fEMCal_dphiEta_BG(0)
{
    // default constructor, don't allocate memory here!
    // this is used by root for IO purposes, it needs to remain empty
}

//_____________________________________________________________________________
AliAnalysisTaskMCInfo::AliAnalysisTaskMCInfo(const char* name,
  Long_t state,
  UInt_t TTmask, UInt_t TTpattern) 
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
  , fGammaE(0)
  , fNeutralPDG(0)
  , fEmcalHitMothers_SIG(0)
  , fEmcalHitMothers_BG(0)
  , fDistBadChannel_SIG(0)
  , fDistBadChannel_BG(0)
  , fEMCnClus_SIG(0)
  , fEMCnClus_BG(0)
  , fEMCnMatchedClus_SIG(0)
  , fEMCnMatchedClus_BG(0)
  , fEMC_nClusVSnMatched_SIG(0)
  , fEMC_nClusVSnMatched_BG(0)
  , fPHOSnClus_SIG(0)
  , fPHOSnClus_BG(0)
  , fPHOSenergy_SIG(0)
  , fPHOSenergy_BG(0)
  , fEMCenergy_SIG(0)
  , fEMCenergy_BG(0)
  , fEMC_nClusVSenergy_SIG(0)
  , fEMC_nClusVSenergy_BG(0)
  , fPHOS_nClusVSenergy_SIG(0)
  , fPHOS_nClusVSenergy_BG(0)
  , fEMCDphi_SIG(0)
  , fEMCDphi_BG(0)
  , fEMCDz_SIG(0)
  , fEMCDz_BG(0)
  , fEMCal_dphiEta_SIG(0)
  , fEMCal_dphiEta_BG(0)
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
AliAnalysisTaskMCInfo::~AliAnalysisTaskMCInfo()
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
void AliAnalysisTaskMCInfo::UserCreateOutputObjects()
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
    fGammaE  = new TH1F("fGammaE",  "fGammaE",  100, 0, 4);       
    fNeutralPDG = new TH1F("fNeutralPDG", "fNeutralPDG", 3500, 0, 3500);
    fEmcalHitMothers_SIG = new TH1F("fEmcalHitMothers_SIG", "fEmcalHitMothers_SIG", 100, 0, 2500);
    fEmcalHitMothers_BG = new TH1F("fEmcalHitMothers_BG", "fEmcalHitMothers_BG", 100, 0, 2500);

    fDistBadChannel_SIG = new TH1F("dBadChannel_sig", "dBadChannel_sig", 100, 0, 50);
    fDistBadChannel_BG = new TH1F("dBadChannel_bg", "dBadChannel_bg", 10, 0, 50);
    fEMCnClus_SIG = new TH1F("EMCnClusters_sig", "EMCnClusters_sig", 10, 0, 10); 
    fEMCnClus_BG = new TH1F("EMCnClusters_bg", "EMCnClusters_bg", 10, 0, 10);  
    fPHOSnClus_SIG = new TH1F("PHOSnClusters_sig", "PHOSnClusters_sig", 10, 0, 10); 
    fPHOSnClus_BG = new TH1F("PHOSnClusters_bg", "PHOSnClusters_bg", 10, 0, 10);   

    fEMCnMatchedClus_SIG = new TH1F("EMCnMatchedClus_sig", "EMCnMatchedClus_sig", 10,0,10);
    fEMCnMatchedClus_BG = new TH1F("EMCnMatchedClus_bg", "EMCnMatchedClus_bg", 10,0,10);
    fEMC_nClusVSnMatched_SIG = new TH2F("EMC_nClusVSnMatched_sig", "EMC_nClusVSnMatched_sig", 
            10,0,10, 10,0,10);
    fEMC_nClusVSnMatched_BG = new TH2F("EMC_nClusVSnMatched_bg", "EMC_nClusVSnMatched_bg", 
            10,0,10, 10,0,10);
    fEMCDphi_SIG = new TH1F("emc_dphi_sig", "emc_dphi_sig", 100,0,3.2);
    fEMCDphi_BG = new TH1F("emc_dphi_bg", "emc_dphi_bg", 100,0,3.2);
    fEMCDz_SIG = new TH1F("emc_dz_sig", "emc_dz_sig", 100,0,3.2);
    fEMCDz_BG = new TH1F("emc_dz_bg", "emc_dz_bg", 100,0,3.2);

    fEMCenergy_SIG = new TH1F("EMCenergy_sig", "EMCenergy_sig", 100, 0, 3);   
    fEMCenergy_BG = new TH1F("EMCenergy_bg", "EMCenergy_bg", 100, 0, 3);     
    fPHOSenergy_SIG = new TH1F("PHOSenergy_sig", "PHOSenergy_sig", 100, 0, 3);    
    fPHOSenergy_BG = new TH1F("PHOSenergy_bg", "PHOSenergy_bg", 100, 0, 3);      
    fEMC_nClusVSenergy_SIG = new TH2F("emc_n_vs_e_sig", "emc_n_vs_e_sig", 10,0,10,100,0,3);
    fEMC_nClusVSenergy_BG = new TH2F("emc_n_vs_e_bg", "emc_n_vs_e_bg", 10,0,10,100,0,3);
    fPHOS_nClusVSenergy_SIG = new TH2F("phos_n_vs_e_sig", "phos_n_vs_e_sig", 10,0,10,100,0,3);
    fPHOS_nClusVSenergy_BG = new TH2F("phos_n_vs_e_bg", "phos_n_vs_e_bg", 10,0,10,100,0,3);

    fEMCal_dphiEta_SIG =  new TH1F("emc_dphieta_sig", "emc_dphieta_sig", 500,0,10);
    fEMCal_dphiEta_BG =  new TH1F("emc_dphieta_bg", "emc_dphieta_bg", 500,0,10);
  
    fOutList->Add(fGammaE);          
    fOutList->Add(fNeutralPDG);          
    fOutList->Add(fEmcalHitMothers_SIG);          
    fOutList->Add(fEmcalHitMothers_BG);          

    fOutList->Add(fDistBadChannel_SIG);          
    fOutList->Add(fDistBadChannel_BG);          
    fOutList->Add(fEMCnClus_SIG);          
    fOutList->Add(fEMCnClus_BG);          
    fOutList->Add(fPHOSnClus_SIG);          
    fOutList->Add(fPHOSnClus_BG);          
    fOutList->Add(fEMCenergy_SIG);          
    fOutList->Add(fEMCenergy_BG);          
    fOutList->Add(fPHOSenergy_SIG);          
    fOutList->Add(fPHOSenergy_BG);          
    fOutList->Add(fEMC_nClusVSenergy_SIG);          
    fOutList->Add(fEMC_nClusVSenergy_BG);          
    fOutList->Add(fPHOS_nClusVSenergy_SIG);          
    fOutList->Add(fPHOS_nClusVSenergy_BG);          

    fOutList->Add(fEMCnMatchedClus_SIG);          
    fOutList->Add(fEMCnMatchedClus_BG);          
    fOutList->Add(fEMC_nClusVSnMatched_SIG);          
    fOutList->Add(fEMC_nClusVSnMatched_BG);          

    fOutList->Add(fEMCDphi_SIG);          
    fOutList->Add(fEMCDphi_BG);          
    fOutList->Add(fEMCDz_SIG);          
    fOutList->Add(fEMCDz_BG);          

    fOutList->Add(fEMCal_dphiEta_SIG);          
    fOutList->Add(fEMCal_dphiEta_BG);          


    PostData(1, fOutList);              // postdata will notify the analysis manager of changes 
                                        // and updates to the fOutList object. 
                                        // the manager will in the end take care of writing 
                                        // the output to file so it needs to know what's 
                                        // in the output
}
//_____________________________________________________________________________
void AliAnalysisTaskMCInfo::UserExec(Option_t *)
{
    // user exec
    // this function is called once for each event
    // the manager will take care of reading the events from file, 
    // and with the static function InputEvent() you have access to the current event. 
    // once you return from the UserExec function, 
    // the manager will retrieve the next event from the chain
    fESD = dynamic_cast<AliESDEvent*>(InputEvent()); 
    if(!fESD) return;
    // here we filter for events that satisfy the condition (filter-bit 107 in CEP evts)
    // 0: remove pileup
    // 1: CCUP13
    // 3: !V0
    // 5: !AD
    // 6: *FO>=1 (to replay OSMB) && *FO<=trks
    // - 2 tracks
    Int_t nTracksAccept(2);
    Int_t nTracks = fCEPUtil->AnalyzeTracks(fESD,fTracks,fTrackStatus);

    TArrayI *TTindices  = new TArrayI();
    Int_t nTracksTT = fCEPUtil->countstatus(fTrackStatus, fTTmask, fTTpattern, TTindices);
    if (nTracksTT!=nTracksAccept) return;
    
    // remove pileup
    if (fESD->IsPileupFromSPD(3,0.8,3.,2.,5.)) return;
    // CCUP13
    Bool_t isV0A = fTrigger->IsOfflineTriggerFired(fESD, AliTriggerAnalysis::kV0A);
    Bool_t isV0C = fTrigger->IsOfflineTriggerFired(fESD, AliTriggerAnalysis::kV0C);
    UInt_t isSTGTriggerFired;
    const AliMultiplicity *mult = (AliMultiplicity*)fESD->GetMultiplicity();
    TBits foMap = mult->GetFastOrFiredChips();
    isSTGTriggerFired  = IsSTGFired(&foMap,0) ? (1<<0) : 0;
    for (Int_t ii=1; ii<=10; ii++) {
        isSTGTriggerFired |= IsSTGFired(&foMap,ii) ? (1<<ii) : 0;
    }
    if (isV0A || isV0C || !isSTGTriggerFired) return;
    // !V0
    // is encorporated in CCUP13
    // !AD
    Bool_t isADA = fTrigger->IsOfflineTriggerFired(fESD, AliTriggerAnalysis::kADA);
    Bool_t isADC = fTrigger->IsOfflineTriggerFired(fESD, AliTriggerAnalysis::kADC);
    if (isADA || isADC) return;
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
    if (!firedChipsOK) return;
  
    // here we have now events which passed the track selection
    
    // get MC event (fMCEvent is member variable from AliAnalysisTaskSE)
    fMCEvent = MCEvent();
    if (fMCEvent) {  
        if (fMCEvent->Stack()==NULL) fMCEvent=NULL;
    }
    AliStack *stack = NULL;
    if (fMCEvent) stack = fMCEvent->Stack();
    else { 
        printf("<E> No MC-event available!\n"); 
        return ;
        /* gSystem->Exit(1); */ 
    }
    // get information if event is fully-reconstructed or not
    Int_t nTracksMC = stack->GetNtrack();
    Int_t nTracksPrimMC = stack->GetNprimary();
    Int_t nTracksTranspMC = stack->GetNtransported();
    /* printf("---------------------------\nNumber of\ntracks: %i\nprimaries: %i\ntransported: %i\n", */ 
    /*         nTracksMC, nTracksPrimMC, nTracksTranspMC); */
    // get lorentzvector of the X particle
    TLorentzVector X_lor = GetXLorentzVector(fMCEvent);
    
    // calculate the lorentzvector of the measured particles and check if they agree with X_lor
    TLorentzVector measured_lor = TLorentzVector(0,0,0,0);
    for (Int_t ii=0; ii<nTracksTT; ii++) {
        // proper pointer into fTracks and fTrackStatus
        Int_t trkIndex = TTindices->At(ii);
        // the original track
        AliESDtrack *tmptrk = (AliESDtrack*) fTracks->At(trkIndex);
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
    if (m_diff < 1e-5) isFullRecon = kTRUE;
    PrintStack(fMCEvent, kFALSE);

    for (Int_t ii=4; ii<nTracksPrimMC; ii++) {
        if (stack->Particle(ii)->GetStatusCode()==0) continue;
        TParticle* part = stack->Particle(ii);
        Int_t pdg = part->GetPdgCode();
        Double_t charge = TDatabasePDG::Instance()->GetParticle(pdg)->Charge();
        if(charge==0.) {
            if (pdg==22) fGammaE->Fill(part->Energy()); 
            fNeutralPDG->Fill(pdg); 
        }
    }
    EMCalAnalysis(isFullRecon, nTracksTT, TTindices);

    PostData(1, fOutList);          // stream the results the analysis of this event to
                                    // the output manager which will take care of writing
                                    // it to a file
}
//_____________________________________________________________________________
void AliAnalysisTaskMCInfo::Terminate(Option_t *)
{
    // terminate
    // called at the END of the analysis (when all events are processed)
}
//_____________________________________________________________________________
//
//
//------------------------------------------------------------------------------
// code to check if the STG trigger had fired
// code from Evgeny Kryshen
// dphiMin/dphiMax specifies the range for the angle between two tracks
Bool_t AliAnalysisTaskMCInfo::IsSTGFired(TBits* fFOmap,Int_t dphiMin,Int_t dphiMax)
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

//_____________________________________________________________________________
TLorentzVector AliAnalysisTaskMCInfo::GetXLorentzVector(AliMCEvent* MCevent)
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
void AliAnalysisTaskMCInfo::PrintStack(AliMCEvent* MCevent, Bool_t prim)
{
    AliStack* stack = MCevent->Stack();
    Int_t nPrimaries = stack->GetNprimary();
    Int_t nTracks = stack->GetNtrack();

    /* TLorentzVector lvtmp; */
    printf("\n-----------------------------------------------\n");
    Int_t nParticles = prim ? nPrimaries : nTracks;
    for (Int_t ii(4); ii<nParticles; ii++) {
        TParticle* part = stack->Particle(ii);
        printf("%i: %-13s: E=%-6.2f, Mother: %i", 
                ii, part->GetName(), part->Energy(), part->GetMother(0));

        if (part->GetStatusCode()==1) printf("   final\n");
        else printf("\n");
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

void AliAnalysisTaskMCInfo::EMCalAnalysis(Bool_t isSignal, Int_t nTracksTT, TArrayI *TTindices)
{
    Int_t nEMCClus(0), nPHOSClus(0), nEMCClus_matched(0);
    Double_t ene(0.), dBadChannel(0.), EMCEne(0.), PHOSEne(0.), 
             dTrackDx(-999.), dTrackDz(-999.), dPhiEtaMin(999.);

    Int_t nClusters = fESD->GetNumberOfCaloClusters();
    for (Int_t ii(0); ii<nClusters; ii++) 
    {
        AliESDCaloCluster *clust = fESD->GetCaloCluster(ii);
        ene = clust->E();
        dBadChannel = clust->GetDistanceToBadChannel();
        // get track information
        dTrackDx = clust->GetTrackDx();
        dTrackDz = clust->GetTrackDz();
        if (isSignal) fDistBadChannel_SIG->Fill(dBadChannel);
        else fDistBadChannel_BG->Fill(dBadChannel);

        // number of clusters on EMC/PHOS
        // deposited energy, ignore matched clusters
        if (clust->IsEMCAL()) {
            nEMCClus++;
            if (clust->GetNTracksMatched()>0) nEMCClus_matched++;
            if (clust->GetNTracksMatched()<=0) { 
                EMCEne += ene;
            }
            if (MatchTracks(clust, nTracksTT, TTindices, dPhiEtaMin)) {
                if (IsClusterFromPDG(clust, 211)) 
                    fEMCal_dphiEta_SIG->Fill(dPhiEtaMin);
                if (IsClusterFromPDG(clust, 22) && !isSignal)
                    fEMCal_dphiEta_BG->Fill(dPhiEtaMin);
            }
        }
        if (clust->IsPHOS()) {
            /* nPHOSClus++; */
            if (clust->GetNTracksMatched()<=0) {
                PHOSEne += ene;
                nPHOSClus++;
            }
        }
    }

    printf("N clusters: %i, N matched clusters: %i\n---------- Evt END ----------\n",
            nEMCClus, nEMCClus_matched);
    // update histograms
    if (isSignal) fEMCDphi_SIG->Fill(dTrackDx);
    else fEMCDphi_BG->Fill(dTrackDx); 

    if (isSignal) fEMCDz_SIG->Fill(dTrackDz);
    else fEMCDz_BG->Fill(dTrackDz); 

    if (isSignal) fEMCnClus_SIG->Fill(nEMCClus);
    else fEMCnClus_BG->Fill(nEMCClus); 

    if (isSignal) fEMCnMatchedClus_SIG->Fill(nEMCClus_matched);
    else fEMCnMatchedClus_BG->Fill(nEMCClus_matched); 

    if (isSignal) fEMC_nClusVSnMatched_SIG->Fill(nEMCClus, nEMCClus_matched);
    else fEMC_nClusVSnMatched_BG->Fill(nEMCClus, nEMCClus_matched);

    if (isSignal) fPHOSnClus_SIG->Fill(nPHOSClus);
    else fPHOSnClus_BG->Fill(nPHOSClus);

    if (isSignal)     fEMCenergy_SIG->Fill(EMCEne);
    else fEMCenergy_BG->Fill(EMCEne); 

    if (isSignal) fPHOSenergy_SIG->Fill(PHOSEne);
    else fPHOSenergy_BG->Fill(PHOSEne);

    if (isSignal) fEMC_nClusVSenergy_SIG->Fill(nEMCClus,EMCEne);
    else fEMC_nClusVSenergy_BG->Fill(nEMCClus,EMCEne); 

    if (isSignal) fPHOS_nClusVSenergy_SIG->Fill(nPHOSClus,PHOSEne);
    else fPHOS_nClusVSenergy_BG->Fill(nPHOSClus,PHOSEne);  
}

Bool_t AliAnalysisTaskMCInfo::MatchTracks(AliESDCaloCluster* clust, Int_t nTracksTT, TArrayI* TTindices, Double_t& dPhiEtaMin)
{
    dPhiEtaMin = 999.;
     
    printf("Distance to track in phi: %-6.2f, in eta: %-6.2f\n", 
            clust->GetTrackDx(), clust->GetTrackDz());
    Float_t x[3];
    clust->GetPosition(x);
    TVector3 v3(x[0], x[1], x[2]);
    // v3 phi is in the range [-pi,pi) -> map it to [0, 2pi)
    Double_t cluster_phi = (v3.Phi()>0.) ? v3.Phi() : v3.Phi() + 2.*TMath::Pi();
    Double_t cluster_eta = v3.Eta();
    printf("EMCal cluster positions: phi: %-6.2f, eta: %-6.2f\n", cluster_phi, cluster_eta);

    for (Int_t kk(0); kk<nTracksTT; kk++) {
        // proper pointer into fTracks and fTrackStatus
        Int_t trkIndex = TTindices->At(kk);
        // the original track
        AliESDtrack *tmptrk = (AliESDtrack*) fTracks->At(trkIndex);
        Int_t MCind = tmptrk->GetLabel();
        AliStack* stack = fMCEvent->Stack();
        if (MCind <= 0) {
            printf("<W> MC index below 0\nCorrection to absolute value!\n"); 
            if (abs(MCind) <= stack->GetNprimary()) MCind = abs(MCind);
            else return kFALSE;
        }
        TParticle* part = stack->Particle(MCind);
        // set MC mass and momentum
        TLorentzVector lv;
        part->Momentum(lv);
        Double_t trkPhiOnEmc = tmptrk->GetTrackPhiOnEMCal();
        // Map phi to [0,2pi)
        trkPhiOnEmc = (trkPhiOnEmc>0.) ? trkPhiOnEmc : trkPhiOnEmc+2.*TMath::Pi();
        if (trkPhiOnEmc<0.) trkPhiOnEmc=-999.;
        Double_t trkEtaOnEmc = tmptrk->GetTrackEtaOnEMCal();

        printf("Track %i: Phi on emcal: %-6.2f, eta: %-6.2f\n",
                kk, trkPhiOnEmc, trkEtaOnEmc);
        if (trkPhiOnEmc==-999. || trkEtaOnEmc==-999.) continue;
        // calculate distance in phi
        Double_t dEta = trkEtaOnEmc - cluster_eta;
        Double_t dPhi = trkPhiOnEmc - cluster_phi;
        Double_t dPhiEta = TMath::Sqrt( dEta*dEta + dPhi*dPhi );

        dPhiEtaMin = (dPhiEtaMin<dPhiEta) ? dPhiEtaMin : dPhiEta;
    }
    printf("Minimum distance to track in eta phi: %-6.2f\n", dPhiEtaMin);
    return (dPhiEtaMin==999.) ? kFALSE : kTRUE;
}

Bool_t AliAnalysisTaskMCInfo::IsClusterFromPDG(AliESDCaloCluster* clust, Int_t pdg)
{
    AliStack* stack = fMCEvent->Stack();
    Int_t MCClusterIndex = clust->GetLabel();
    Int_t particle_pdg = abs(stack->Particle(MCClusterIndex)->GetPdgCode());
    // we check if the cluster is made by a scondary originating from a primary particle
    Int_t mc_idx = MCClusterIndex;
    while (!stack->IsPhysicalPrimary(mc_idx)) {
        // check if mother particle is primary
        mc_idx = stack->Particle(mc_idx)->GetMother(0);
        particle_pdg = abs(stack->Particle(mc_idx)->GetPdgCode());
    }
    printf("Primary particle causing cluster: %i (pdg: %i)\n", mc_idx, particle_pdg); 
    return (particle_pdg==pdg);
}
