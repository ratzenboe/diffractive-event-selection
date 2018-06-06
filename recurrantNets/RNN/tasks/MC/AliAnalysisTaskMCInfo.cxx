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
#include "AliAnalysisTaskMCInfo.h"

#include <string>         // std::string
#include <cstddef>         // std::size_t

class AliAnalysisTaskMCInfo;    // your analysis class

using namespace std;            // std namespace: so you can do things like 'cout'

ClassImp(AliAnalysisTaskMCInfo) // classimp: necessary for root

AliAnalysisTaskMCInfo::AliAnalysisTaskMCInfo() 
  : AliAnalysisTaskSE()
  , CEPBGBase(AliCEPBase::kBitConfigurationSet,
              AliCEPBase::kTTBaseLine,
              AliCEPBase::kTTBaseLine,
              "EMCAL.Hits.root")
  , fOutList(0)
  , fGammaE(0)
  , fEMCalSecondaryE_SIG(0)
  , fEMCalSecondaryE_BG(0)
  , fEMCnClus_SIG(0)
  , fEMCnClus_BG(0)
  , fEMCnMatchedClus_SIG(0)
  , fEMCnMatchedClus_BG(0)
  , fEMC_nClusVSnMatched_SIG(0)
  , fEMC_nClusVSnMatched_BG(0)
  , fEMCenergy_SIG(0)
  , fEMCenergy_BG(0)
  , fEMC_nClusVSenergy_SIG(0)
  , fEMC_nClusVSenergy_BG(0)
  , fEMCal_dphiEta_pion(0)
  , fEMCal_dphiEta_gamma(0)
{
    // default constructor, don't allocate memory here!
    // this is used by root for IO purposes, it needs to remain empty
}

//_____________________________________________________________________________
AliAnalysisTaskMCInfo::AliAnalysisTaskMCInfo(const char* name,
  Long_t state,
  UInt_t TTmask, 
  UInt_t TTpattern,
  TString hitFileName) 
  : AliAnalysisTaskSE(name)
  , CEPBGBase(state,
              TTmask,
              TTpattern,
              hitFileName)
  , fOutList(0)
  , fGammaE(0)
  , fEMCalSecondaryE_SIG(0)
  , fEMCalSecondaryE_BG(0)
  , fEMCnClus_SIG(0)
  , fEMCnClus_BG(0)
  , fEMCnMatchedClus_SIG(0)
  , fEMCnMatchedClus_BG(0)
  , fEMC_nClusVSnMatched_SIG(0)
  , fEMC_nClusVSnMatched_BG(0)
  , fEMCenergy_SIG(0)
  , fEMCenergy_BG(0)
  , fEMC_nClusVSenergy_SIG(0)
  , fEMC_nClusVSenergy_BG(0)
  , fEMCal_dphiEta_pion(0)
  , fEMCal_dphiEta_gamma(0)
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
    fEvtStorge = EventStorage();

    fRunNumber = fCurrentRunNumber;

    fHitsArray = new TClonesArray("AliEMCALHit",1000);

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
    fEMCalSecondaryE_SIG = new TH1F("fEMCalSecondaryE_SIG", "fEMCalSecondaryE_SIG", 100, 0, 4);
    fEMCalSecondaryE_BG  = new TH1F("fEMCalSecondaryE_BG",  "fEMCalSecondaryE_BG",  100, 0, 4);       
    fEMCnClus_SIG = new TH1F("EMCnClusters_sig", "EMCnClusters_sig", 10, 0, 10); 
    fEMCnClus_BG = new TH1F("EMCnClusters_bg", "EMCnClusters_bg", 10, 0, 10);  

    fEMCnMatchedClus_SIG = new TH1F("EMCnMatchedClus_sig", "EMCnMatchedClus_sig", 10,0,10);
    fEMCnMatchedClus_BG = new TH1F("EMCnMatchedClus_bg", "EMCnMatchedClus_bg", 10,0,10);

    fEMC_nClusVSnMatched_SIG = new TH2F("EMC_nClusVSnMatched_sig", "EMC_nClusVSnMatched_sig", 
            10,0,10, 10,0,10);
    fEMC_nClusVSnMatched_BG = new TH2F("EMC_nClusVSnMatched_bg", "EMC_nClusVSnMatched_bg", 
            10,0,10, 10,0,10);

    fEMCenergy_SIG = new TH1F("EMCenergy_sig", "EMCenergy_sig", 100, 0, 3);   
    fEMCenergy_BG = new TH1F("EMCenergy_bg", "EMCenergy_bg", 100, 0, 3);     
    fEMC_nClusVSenergy_SIG = new TH2F("emc_n_vs_e_sig", "emc_n_vs_e_sig", 10,0,10,100,0,3);
    fEMC_nClusVSenergy_BG = new TH2F("emc_n_vs_e_bg", "emc_n_vs_e_bg", 10,0,10,100,0,3);

    fEMCal_dphiEta_pion =  new TH1F("emc_dphieta_pion", "emc_dphieta_pion", 100,0,6);
    fEMCal_dphiEta_gamma =  new TH1F("emc_dphieta_gamma", "emc_dphieta_gamma", 100,0,6);
  
    fOutList->Add(fGammaE);          
    fOutList->Add(fEMCalSecondaryE_SIG);          
    fOutList->Add(fEMCalSecondaryE_BG);          

    fOutList->Add(fEMCnClus_SIG);          
    fOutList->Add(fEMCnClus_BG);          

    fOutList->Add(fEMCnMatchedClus_SIG);          
    fOutList->Add(fEMCnMatchedClus_BG);          

    fOutList->Add(fEMC_nClusVSnMatched_SIG);          
    fOutList->Add(fEMC_nClusVSnMatched_BG);          

    fOutList->Add(fEMCenergy_SIG);          
    fOutList->Add(fEMCenergy_BG);          

    fOutList->Add(fEMC_nClusVSenergy_SIG);          
    fOutList->Add(fEMC_nClusVSenergy_BG);          
 
    fOutList->Add(fEMCal_dphiEta_pion);          
    fOutList->Add(fEMCal_dphiEta_gamma);          

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

    // set global variables
    Int_t nTracks = fCEPUtil->AnalyzeTracks(fESD, fTracks, fTrackStatus);
    Int_t nTracksAccept(2), nTracksTT;
    TArrayI *TTindices  = new TArrayI();
    Bool_t isGoodEvt = lhc16filter(fESD, nTracksAccept, nTracksTT, TTindices);
    if (!isGoodEvt) return ;
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

    //////////////////////////////////////////////////////////////////////////////////
    // ---------------------- Print the event stack ----------------------------------
    /* PrintStack(fMCEvent, kFALSE); */
    // ---------------------- Print the AliESDtracks ---------------------------------
    /* PrintTracks(fESD); */
    //////////////////////////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////////////////////////
    // ----------------------- EMCAL Hits --------------------------------------------
    // the directory may have changed we therefore update the neccessary global vars
    if (UpdateGlobalVars()) PrintEMCALHits(isFullRecon);
    //////////////////////////////////////////////////////////////////////////////////

    EventDef ed;
    for (Int_t ii=4; ii<nTracksPrimMC; ii++) {
        TParticle* part = stack->Particle(ii);

        ed.AddTrack(ii, part->GetPdgCode(), part->GetMother(0), 
                part->GetDaughter(0), part->GetDaughter(1), 
                (part->GetStatusCode()==0) ? kFALSE : kTRUE);

        if (part->GetStatusCode()==0) continue;
        Int_t pdg = part->GetPdgCode();
        Double_t charge = TDatabasePDG::Instance()->GetParticle(pdg)->Charge();
        if (charge==0.) {
            if (pdg==22) fGammaE->Fill(part->Energy()); 
            fNeutralPDG->Fill(pdg); 
        }
    }
    // event storage specific actions
    ed.FinalizeEvent();
    fEvtStorge.AddEvent(ed);

    EMCalAnalysis(isFullRecon, nTracksTT, TTindices);

    /* printf("\n\n\n\n----------------------------------------------------\n\n"); */
    PostData(1, fOutList);          // stream the results the analysis of this event to
                                    // the output manager which will take care of writing
                                    // it to a file
}
//_____________________________________________________________________________
void AliAnalysisTaskMCInfo::Terminate(Option_t *)
{
    // terminate
    // called at the END of the analysis (when all events are processed)
    fEvtStorge->PrintNEvts();
}

//_____________________________________________________________________________
void AliAnalysisTaskMCInfo::EMCalAnalysis(Bool_t isSignal, AliMCEvent* MCevt, TObjArray* tracks,
                                          Int_t nTracksTT, TArrayI *TTindices)
{
    Int_t nEMCClus(0), nEMCClus_matched(0), partpdg(0);
    Double_t ene(0.), dBadChannel(0.), EMCEne(0.), PHOSEne(0.), dPhiEtaMin(999.);

    Int_t nClusters = fESD->GetNumberOfCaloClusters();
    for (Int_t ii(0); ii<nClusters; ii++) 
    {
        AliESDCaloCluster *clust = fESD->GetCaloCluster(ii);
        ene = clust->E();
        // number of clusters on EMC
        // deposited energy, ignore matched clusters
        if (clust->IsEMCAL()) {
            nEMCClus++;
            // we match clusters ourselves
            /* if (clust->GetNTracksMatched()>0) nEMCClus_matched++; */
            /* if (clust->GetNTracksMatched()<=0) EMCEne += ene; */
            if (MatchTracks(clust, tracks, nTracksTT, TTindices, MCevt, dPhiEtaMin)) {
                partpdg = 211;
                if (IsClusterFromPDG(clust, partpdg, MCevt)) 
                    fEMCal_dphiEta_pion->Fill(dPhiEtaMin);
                partpdg = 22;
                if (IsClusterFromPDG(clust, partpdg, MCevt) && !isSignal)
                    fEMCal_dphiEta_gamma->Fill(dPhiEtaMin);
            }
        }
    }
    // update histograms
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

//_____________________________________________________________________________

Bool_t AliAnalysisTaskMCInfo::MatchTracks(AliESDCaloCluster* clust, Int_t nTracksTT, TArrayI* TTindices, Double_t& dPhiEtaMin)
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

        /* printf("Track %i: Phi on emcal: %-6.2f, eta: %-6.2f\n", */
        /*         kk, trkPhiOnEmc, trkEtaOnEmc); */
        if (trkPhiOnEmc==-999. || trkEtaOnEmc==-999.) continue;
        // calculate distance in phi
        Double_t dEta = trkEtaOnEmc - cluster_eta;
        Double_t dPhi = trkPhiOnEmc - cluster_phi;
        Double_t dPhiEta = TMath::Sqrt( dEta*dEta + dPhi*dPhi );

        dPhiEtaMin = (dPhiEtaMin<dPhiEta) ? dPhiEtaMin : dPhiEta;
    }
    /* printf("Minimum distance to track in eta phi: %-6.2f\n", dPhiEtaMin); */
    return (dPhiEtaMin==999.) ? kFALSE : kTRUE;
}

//_____________________________________________________________________________
Bool_t AliAnalysisTaskMCInfo::IsClusterFromPDG(AliESDCaloCluster* clust, Int_t& pdg)
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
    Bool_t isclusterfrompart = (particle_pdg==pdg);
    pdg = particle_pdg;
    /* printf("Primary particle causing cluster: %i (pdg: %i)\n", mc_idx, particle_pdg); */ 
    return isclusterfrompart;
}

//_____________________________________________________________________________
TString AliAnalysisTaskMCInfo::GetDirFromFullPath(const char* fullpath)
{
    std::string curr_dir(fullpath);
    // find last occurance of "/" (determining the full path to the directory)
    std::size_t path_end_idx = curr_dir.find_last_of("/\\");
    // take only the substring to the last occurance (+1 to also get the slash /)
    TString outstr((curr_dir.substr(0,path_end_idx+1)).c_str());

    return outstr;
}

//_____________________________________________________________________________
Bool_t AliAnalysisTaskMCInfo::UpdateGlobalVars()
{
    if (fCurrentDir=="" || fCurrentDir != GetDirFromFullPath(CurrentFileName())) {
        fCurrentDir = GetDirFromFullPath(CurrentFileName());
        fHitFile = TFile::Open(fCurrentDir+fHitFileName);
    }
    if (!fHitFile) return kFALSE;
    TString iev_str;
    iev_str.Form("Event%i", Entry());
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
void AliAnalysisTaskMCInfo::PrintEMCALHits(Bool_t isSignal)
{
    // For a comparion with the original particle we need the AliStack
    /* AliStack* stack = fMCEvent->Stack(); */
    /* Int_t nTransported = stack->GetNtransported(); */
    /* printf("\n\nNumber of transported tracks: %i\n", nTransported); */
    /* TParticle* part = 0x0; */

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

            if (isSignal) fEMCalSecondaryE_SIG->Fill(hit->GetIenergy());
            else fEMCalSecondaryE_BG->Fill(hit->GetIenergy());
        }
    }
    fHitsArray->Clear();
}

