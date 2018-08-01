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

#include "TRandom.h"
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
#include "AliESDCaloCells.h"
#include "AliESDtrackCuts.h"
#include "AliMultiplicitySelectionCP.h"
#include "AliMC.h"
#include "AliRunLoader.h"

#include "AliCEPBase.h"
/* #include "AliCEPUtils.h" */
#include "AliAnalysisTaskEMCAL.h"

#include <string>         // std::string
#include <cstddef>         // std::size_t

class AliAnalysisTaskEMCAL;    // your analysis class

using namespace std;            // std namespace: so you can do things like 'cout'

ClassImp(AliAnalysisTaskEMCAL) // classimp: necessary for root

AliAnalysisTaskEMCAL::AliAnalysisTaskEMCAL() 
  : AliAnalysisTaskSE()
  , CEPBGBase(AliCEPBase::kBitConfigurationSet,
              AliCEPBase::kTTBaseLine,
              AliCEPBase::kTTBaseLine,
              "EMCAL.Hits.root")
  // nb of events info
  , fAllEvts(0)
  , fnEvtsPassedFilter(0)
  , fnEvtsTotal(0)
  , fnFinishedAnalysis(0)
  , fOutList(0)
  , fGammaE(0)
  , fSecondaryE_SIG(0)
  , fSecondaryE_BG(0)
  , fnCluster_SIG(0)
  , fnCluster_BG(0)
  , fnMatchedCluster_SIG(0)
  , fnMatchedCluster_BG(0)
  , fnClus_VS_nMatched_SIG(0)
  , fnClus_VS_nMatched_BG(0)
  , fEnergy_SIG(0)
  , fEnergy_BG(0)
  , fnClus_VS_energy_SIG(0)
  , fnClus_VS_energy_BG(0)
  , fdPhiEta_pion(0)
  , fdPhiEta_gamma(0)
{
    // default constructor, don't allocate memory here!
    // this is used by root for IO purposes, it needs to remain empty
}

//_____________________________________________________________________________
AliAnalysisTaskEMCAL::AliAnalysisTaskEMCAL(const char* name,
  Long_t state,
  UInt_t TTmask, 
  UInt_t TTpattern,
  TString hitFileName) 
  : AliAnalysisTaskSE(name)
  , CEPBGBase(state,
              TTmask,
              TTpattern,
              hitFileName)
  // nb of events info
  , fAllEvts(0)
  , fnEvtsPassedFilter(0)
  , fnEvtsTotal(0)
  , fnFinishedAnalysis(0)
  , fOutList(0)
  , fGammaE(0)
  , fSecondaryE_SIG(0)
  , fSecondaryE_BG(0)
  , fnCluster_SIG(0)
  , fnCluster_BG(0)
  , fnMatchedCluster_SIG(0)
  , fnMatchedCluster_BG(0)
  , fnClus_VS_nMatched_SIG(0)
  , fnClus_VS_nMatched_BG(0)
  , fEnergy_SIG(0)
  , fEnergy_BG(0)
  , fnClus_VS_energy_SIG(0)
  , fnClus_VS_energy_BG(0)
  , fdPhiEta_pion(0)
  , fdPhiEta_gamma(0)
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
AliAnalysisTaskEMCAL::~AliAnalysisTaskEMCAL()
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
void AliAnalysisTaskEMCAL::UserCreateOutputObjects()
{
    // create output objects
    //
    // this function is called ONCE at the start of the analysis (RUNTIME)
    // here the histograms and other objects are created
    Initialize();
    // set random seed -> important for pid & event-selection -> want reproducable results
    gRandom->SetSeed(7);
   
    // the histograms are added to a tlist which is in the end saved to an output file
    fOutList = new TList();             // this is a list which will contain all of your histograms
    fOutList->SetOwner(kTRUE);          // memory stuff: the list is owner of all objects 
                                        // it contains and will delete them if requested 
    fGammaE  = new TH1F("fGammaE",  "fGammaE",  100, 0, 4);       
    fSecondaryE_SIG = new TH1F("fSecondaryE_SIG", "fSecondaryE_SIG", 100, 0, 4);
    fSecondaryE_BG  = new TH1F("fSecondaryE_BG",  "fSecondaryE_BG",  100, 0, 4);       
    fnCluster_SIG = new TH1F("fnCluster_SIG", "fnCluster_SIG", 10, 0, 10); 
    fnCluster_BG = new TH1F("fnCluster_BG", "fnCluster_BG", 10, 0, 10);  

    fnMatchedCluster_SIG = new TH1F("fnMatchedCluster_SIG", "fnMatchedCluster_SIG", 10,0,10);
    fnMatchedCluster_BG = new TH1F("fnMatchedCluster_BG", "fnMatchedCluster_BG", 10,0,10);

    fnClus_VS_nMatched_SIG = new TH2F("fnClus_VS_nMatched_SIG", "fnClus_VS_nMatched_SIG", 
            10,0,10, 10,0,10);
    fnClus_VS_nMatched_BG = new TH2F("fnClus_VS_nMatched_BG", "fnClus_VS_nMatched_BG", 
            10,0,10, 10,0,10);

    fEnergy_SIG = new TH1F("fEnergy_SIG", "fEnergy_SIG", 100, 0, 3);   
    fEnergy_BG = new TH1F("fEnergy_BG", "fEnergy_BG", 100, 0, 3);     
    fnClus_VS_energy_SIG= new TH2F("fnClus_VS_energy_SIG","fnClus_VS_energy_SIG", 10,0,10,100,0,3);
    fnClus_VS_energy_BG = new TH2F("fnClus_VS_energy_BG", "fnClus_VS_energy_BG", 10,0,10,100,0,3);

    fdPhiEta_pion =  new TH1F("fdPhiEta_pion", "fdPhiEta_pion", 100,0,6);
    fdPhiEta_gamma =  new TH1F("fdPhiEta_gamma", "fdPhiEta_gamma", 100,0,6);

    fClusterTime_SIG =  new TH1F("fClusterTime_SIG", "fClusterTime_SIG", 100,-1.e-7,1.e-7);
    fClusterTime_BG  =  new TH1F("fClusterTime_BG",  "fClusterTime_BG",  100,-1.e-7,1.e-7);
  
    fOutList->Add(fGammaE);          
    fOutList->Add(fSecondaryE_SIG);          
    fOutList->Add(fSecondaryE_BG);          

    fOutList->Add(fnCluster_SIG);          
    fOutList->Add(fnCluster_BG);          

    fOutList->Add(fnMatchedCluster_SIG);          
    fOutList->Add(fnMatchedCluster_BG);          

    fOutList->Add(fnClus_VS_nMatched_SIG);          
    fOutList->Add(fnClus_VS_nMatched_BG);          

    fOutList->Add(fEnergy_SIG);          
    fOutList->Add(fEnergy_BG);          

    fOutList->Add(fnClus_VS_energy_SIG);          
    fOutList->Add(fnClus_VS_energy_BG);          
 
    fOutList->Add(fdPhiEta_pion);          
    fOutList->Add(fdPhiEta_gamma);          

    fOutList->Add(fClusterTime_SIG);          
    fOutList->Add(fClusterTime_BG);          

    PostData(1, fOutList);              // postdata will notify the analysis manager of changes 
                                        // and updates to the fOutList object. 
                                        // the manager will in the end take care of writing 
                                        // the output to file so it needs to know what's 
                                        // in the output
}

//_____________________________________________________________________________
void AliAnalysisTaskEMCAL::UserExec(Option_t *)
{
    fAllEvts++;
    // user exec
    // this function is called once for each event
    // the manager will take care of reading the events from file, 
    // and with the static function InputEvent() you have access to the current event. 
    // once you return from the UserExec function, 
    // the manager will retrieve the next event from the chain
    if (!fInputHandler) { printf("<E> no input handler available!\n"); return ; }
    // fESD is (protected) member of CEPBGBase
    fESD = dynamic_cast<AliESDEvent*>(InputEvent()); 
    if(!fESD) return;
    // check pid-response (protected member of CEPBGBase)
    fPIDResponse = (AliPIDResponse*)fInputHandler->GetPIDResponse();
    if (!fPIDResponse) { printf("<E> no PID available!\n"); return ; }

    // ///////////////////////////////////////////////////////////////////////////////
    // get MC event (member of AliAnalysisTaskSE)
    fMCEvent = MCEvent();
    if (fMCEvent) { if (fMCEvent->Stack()==NULL) fMCEvent=NULL; }
    AliStack *stack = NULL;
    if (fMCEvent) stack = fMCEvent->Stack();
    else { printf("<E> No MC-event available!\n"); return ; }
    // ///////////////////////////////////////////////////////////////////////////////
    fnEvtsTotal++;
    
    // ///////////////////////////////////////////////////////////////////////////////
    // ------------------------- event filter ----------------------------------------
    Int_t nTracks = fCEPUtil->AnalyzeTracks(fESD, fTracks, fTrackStatus); 
    Int_t nTracksTT, nTracksAccept=2;
    TArrayI *TTindices  = new TArrayI();
    Bool_t isGoodEvt = lhc16filter(fESD,nTracksAccept,fTTmask,fTTpattern,nTracksTT,TTindices); 
    if (!isGoodEvt) return ;
    std::vector<Int_t> good_evt_particles = {211, -211};
    if (!HasRightParticles(fTracks, nTracksTT, TTindices, 
                           fPIDResponse, fPIDCombined, 
                           fMCEvent, good_evt_particles)) return ;
    fnEvtsPassedFilter++;
    // ///////////////////////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////////////////////////
    // ---------------------- Print the event stack ----------------------------------
    /* PrintStack(fMCEvent, kFALSE); */
    // ---------------------- Print the AliESDtracks ---------------------------------
    /* PrintTracks(fTracks, nTracksTT, TTindices); */
    /* printf("----------------------------- evt end ---------------------------------\n\n"); */
    //////////////////////////////////////////////////////////////////////////////////
    
    Bool_t isSignal = EvtFullRecon(fTracks, nTracksTT, TTindices, fMCEvent);  
    EMCalAnalysis(isSignal, fMCEvent, fTracks, nTracksTT, TTindices);
    //////////////////////////////////////////////////////////////////////////////////
    // ----------------------- EMCAL Hits --------------------------------------------
    // the directory may have changed we therefore update the neccessary global vars
    /* if (UpdateGlobalVars(CurrentFileName(), Entry())) EMCalHits(isSignal); */
    /* else { printf("<E> global varaibles cannot be updated!\n"); return; } */
    //////////////////////////////////////////////////////////////////////////////////

    fnFinishedAnalysis++;

    /* printf("\n\n\n\n----------------------------------------------------\n\n"); */
    PostData(1, fOutList);          // stream the results the analysis of this event to
                                    // the output manager which will take care of writing
                                    // it to a file
}

//_____________________________________________________________________________
void AliAnalysisTaskEMCAL::Terminate(Option_t *)
{
    // terminate
    // called at the END of the analysis (when all events are processed)
    /* fEvtStorge.PrintNEvts(); */
    printf("%i/%i events are usable\n", fnEvtsTotal, fAllEvts);
    printf("%i/%i passed lhc16filter\n", fnEvtsPassedFilter, fnEvtsTotal);
    printf("%i/%i passed UpdateGlobalVars\n", fnFinishedAnalysis, fnEvtsPassedFilter);
}

//_____________________________________________________________________________
void AliAnalysisTaskEMCAL::EMCalAnalysis(Bool_t isSignal, AliMCEvent* MCevt, 
                TObjArray* tracks, Int_t nTracksTT, TArrayI *TTindices)
{
    Int_t nCluster(0), nClusMatched(0), partpdg(0);
    Double_t energy(0.), dPhiEtaMin(999.);

    Int_t nClusters = fESD->GetNumberOfCaloClusters();
    for (Int_t ii(0); ii<nClusters; ii++) 
    {
        AliESDCaloCluster *clust = fESD->GetCaloCluster(ii);
        if (!clust->IsEMCAL()) continue;
        // ///////////////////////////////////////////////////////////////////////////////
        // check timing info:
        if (clust->GetNCells()>0) 
        {
            AliESDCaloCells* caloCells = (AliESDCaloCells*)fESD->GetEMCALCells();
            // variable preperation for easier readability
            Double_t cellAmpl_max, cellAmpl;
            Short_t cellNb, cellNb_max_ampl;
            // minimum cell ampl = 0
            cellAmpl_max = 0.;
            // Looping thru all cells that fired 
            for (UInt_t kk(0); kk<clust->GetNCells(); kk++) {
                cellNb = clust->GetCellAbsId(kk);
                cellAmpl = caloCells->GetCellAmplitude(cellNb);
                if (cellAmpl>=cellAmpl_max) { cellAmpl_max = cellAmpl; cellNb_max_ampl = cellNb; }
            }
            // fill the cell time of the maximum energy cluster
            if (isSignal) fClusterTime_SIG->Fill(caloCells->GetCellTime(cellNb_max_ampl));
            else fClusterTime_BG->Fill(caloCells->GetCellTime(cellNb_max_ampl));
        }
        // ///////////////////////////////////////////////////////////////////////////////

        // ///////////////////////////////////////////////////////////////////////////////
        // number of clusters on EMC
        nCluster++;
        // ///////////////////////////////////////////////////////////////////////////////
        // we match clusters ourselves
        dPhiEtaMin = 999.;
        if (MatchTracks(clust, tracks, nTracksTT, TTindices, dPhiEtaMin)) {
            partpdg = 211;
            if (IsClusterFromPDG(clust, partpdg, MCevt)) 
                fdPhiEta_pion->Fill(dPhiEtaMin);
            partpdg = 22;
            if (IsClusterFromPDG(clust, partpdg, MCevt) && !isSignal)
                fdPhiEta_gamma->Fill(dPhiEtaMin);
        }
        if (dPhiEtaMin<0.51) nClusMatched++;
        // only plot energy that comes not from a cluster
        // we want the energy in general not from matched cluters
        /* else energy += clust->E(); */
        // ///////////////////////////////////////////////////////////////////////////////
        energy += clust->E();
    }
        
    // ///////////////////////////////////////////////////////////////////////////////
    // ---------------------------- update histograms --------------------------------
    // number of clusters
    if (isSignal) fnCluster_SIG->Fill(nCluster);
    else fnCluster_BG->Fill(nCluster); 
    // number of matched clusters
    if (isSignal) fnMatchedCluster_SIG->Fill(nClusMatched);
    else fnMatchedCluster_BG->Fill(nClusMatched); 
    // 2D number of clusters vs number of matched clusters
    if (isSignal) fnClus_VS_nMatched_SIG->Fill(nCluster, nClusMatched);
    else fnClus_VS_nMatched_BG->Fill(nCluster, nClusMatched);
    // deposited energy
    if (isSignal) fEnergy_SIG->Fill(energy);
    else fEnergy_BG->Fill(energy); 
    // 2D number of clusters vs deposited energy
    if (isSignal) fnClus_VS_energy_SIG->Fill(nCluster,energy);
    else fnClus_VS_energy_BG->Fill(nCluster,energy); 
    // ///////////////////////////////////////////////////////////////////////////////
    
    // ///////////////////////////////////////////////////////////////////////////////
    // Energy available in primary produced gammas
    AliStack* stack = MCevt->Stack();
    Int_t nTracksPrimMC = stack->GetNprimary();

    for (Int_t ii=4; ii<nTracksPrimMC; ii++) {
        TParticle* part = stack->Particle(ii);
        if (part->GetStatusCode()==0) continue;
        Int_t pdg = part->GetPdgCode();
        Double_t charge = TDatabasePDG::Instance()->GetParticle(pdg)->Charge();
        if (charge==0.) {
            if (pdg==22) fGammaE->Fill(part->Energy()); 
        }
    }
    // ///////////////////////////////////////////////////////////////////////////////
}

void AliAnalysisTaskEMCAL::EMCalHits(Bool_t isSignal)
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

            parent_v.push_back(hit->GetIparent());

            if (isSignal) fSecondaryE_SIG->Fill(hit->GetIenergy());
            else fSecondaryE_BG->Fill(hit->GetIenergy());
        }
    }
    fHitsArray->Clear();
}

