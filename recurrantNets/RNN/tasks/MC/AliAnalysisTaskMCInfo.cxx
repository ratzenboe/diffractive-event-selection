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
  , fEmcalHitMothers(0)
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
  , fEmcalHitMothers(0)
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
    fGammaE  = new TH1F("fGammaE",  "fGammaE",  100, 0, 10);       
    fNeutralPDG = new TH1F("fNeutralPDG", "fNeutralPDG", 3500, 0, 3500);
    fEmcalHitMothers = new TH1F("fEmcalHitMothers", "fEmcalHitMothers", 100, 0, 2500);
    fDistBadChannel_SIG;
    fDistBadChannel_BG;
    fEMCnClus_SIG;
    fEMCnClus_BG;  
    fPHOSnClus_SIG; 
    fPHOSnClus_BG;   
    fEMCenergy_SIG;   
    fEMCenergy_BG;     
    fPHOSenergy_SIG;    
    fPHOSenergy_BG;      
    fEMC_nClusVSenergy_SIG;
    fEMC_nClusVSenergy_BG; 
    fPHOS_nClusVSenergy_SIG;
    fPHOS_nClusVSenergy_BG;  
  
    fOutList->Add(fGammaE);          
    fOutList->Add(fNeutralPDG);          
    fOutList->Add(fEmcalHitMothers);          

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
    /* PrintStack(fMCEvent, kFALSE); */

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
    for (Int_t ii=4; ii<nParticles; ii++) {
        TParticle* part = stack->Particle(ii);
        printf("%i: %-13s: E=%-6.2f", ii, part->GetName(), part->Energy());
        if (part->GetStatusCode()==1) printf("   final\n");
        else printf("\n");
        if (ii==nPrimaries-1) printf("-------------- Primaries end ------------------\n");
        /* if (stack->Particle(ii)->GetMother(0)==0) { */
    }
    printf("-----------------------------------------------\n");
    return ;
}

void AliAnalysisTaskMCInfo::EMCalAnalysis(void)
{
    Int_t nEMCClus(0), nPHOSClus(0);
    Double_t ene(0.), dBadChannel(0.), EMCEne(0.), PHOSEne(0.);

    Int_t nClusters = fESD->GetNumberOfCaloClusters();
    for (Int_t ii(0); ii<nClusters; ii++) 
    {
        AliESDCaloCluster *clust = fESD->GetCaloCluster(ii);
        ene = clust->E();
        dBadChannel = clust->GetDistanceToBadChannel();
        if (isSignal) fDistBadChannel_SIG->Fill(dBadChannel);
        else fDistBadChannel_BG->Fill(dBadChannel);

        // number of clusters on EMC/PHOS
        // deposited energy, ignore matched clusters
        if (clust->IsEMCAL()) {
            nEMCClus++;
            if (clust->GetNTracksMatched()<=0) EMCEne += ene;
        }
        if (clust->IsPHOS()) {
            nPHOSClus++;
            if (clust->GetNTracksMatched()<=0) PHOSEne += ene;
        }
    }

    // update histograms
    if (isSignal) fEMCnClus_SIG->Fill(nEMCClus);
    else fEMCnClus_BG->Fill(nEMCClus); 

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
