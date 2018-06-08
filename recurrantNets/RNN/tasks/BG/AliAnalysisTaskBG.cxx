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
#include "AliESDtrackCuts.h"
#include "AliMultiplicitySelectionCP.h"
#include "AliMC.h"
#include "AliRunLoader.h"

#include "AliCEPBase.h"
/* #include "AliCEPUtils.h" */
#include "AliAnalysisTaskBG.h"

#include <string>         // std::string
#include <cstddef>         // std::size_t

class AliAnalysisTaskBG;    // your analysis class

using namespace std;            // std namespace: so you can do things like 'cout'

ClassImp(AliAnalysisTaskBG) // classimp: necessary for root

AliAnalysisTaskBG::AliAnalysisTaskBG() 
  : AliAnalysisTaskSE()
  , CEPBGBase(AliCEPBase::kBitConfigurationSet,
              AliCEPBase::kTTBaseLine,
              AliCEPBase::kTTBaseLine,
              "EMCAL.Hits.root")
  , fOutList(0)
  , fInvMass_FD(0)
  // BG 3+ study
  , fInvMass_3trks(0)
  , fInvMass_3plusTrks(0)
  // Gamma BG study
  , fInvMass_GammaDet(0)
{
    // default constructor, don't allocate memory here!
    // this is used by root for IO purposes, it needs to remain empty
}

//_____________________________________________________________________________
AliAnalysisTaskBG::AliAnalysisTaskBG(const char* name,
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
  , fInvMass_FD(0)
  // BG 3+ study
  , fInvMass_3trks(0)
  , fInvMass_3plusTrks(0)
  // Gamma BG study
  , fInvMass_GammaDet(0)
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
AliAnalysisTaskBG::~AliAnalysisTaskBG()
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
void AliAnalysisTaskBG::UserCreateOutputObjects()
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
    fInvMass_FD    = new TH1F("fInvMass_FD", "fInvMass_FD",100, 0, 3);
    // BG 3+
    fInvMass_3trks = new TH1F("fInvMass_3trks", "fInvMass_3trks",100, 0, 3);
    fInvMass_3plusTrks = new TH1F("fInvMass_3plusTrks", "fInvMass_3plusTrks",100, 0, 3);
    // BG gamma hit
    fInvMass_GammaDet = new TH1F("fInvMass_GammaDet", "fInvMass_GammaDet",100, 0, 3);

    fOutList->Add(fInvMass_FD);          
    fOutList->Add(fInvMass_3trks);          
    fOutList->Add(fInvMass_3plusTrks);          
    fOutList->Add(fInvMass_GammaDet);          

    PostData(1, fOutList);              // postdata will notify the analysis manager of changes 
                                        // and updates to the fOutList object. 
                                        // the manager will in the end take care of writing 
                                        // the output to file so it needs to know what's 
                                        // in the output
}

//_____________________________________________________________________________
void AliAnalysisTaskBG::UserExec(Option_t *)
{
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
    
    // ///////////////////////////////////////////////////////////////////////////////
    // ------------------------- event filter ----------------------------------------
    Int_t nTracks = fCEPUtil->AnalyzeTracks(fESD, fTracks, fTrackStatus); 
    Int_t nTracksTT;
    std::vector<Int_t> nTracksAccept = { 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    TArrayI *TTindices  = new TArrayI();
    Bool_t isGoodEvt = lhc16filter(fESD,nTracksAccept,fTTmask,fTTpattern,nTracksTT,TTindices); 
    if (!isGoodEvt) return ;
    /* std::vector<Int_t> good_evt_particles = {211, -211}; */
    Int_t pdg_pass = 211;
    if (!HasRightParticles(fTracks, nTracksTT, TTindices, 
                           fPIDResponse, fPIDCombined, 
                           fMCEvent, pdg_pass)) return ;
    // ///////////////////////////////////////////////////////////////////////////////
    // ---------------------------- Analyse type of decay ----------------------------
    if(nTracksTT==2) NewEvent(fMCEvent);
    //////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////
    // ---------------------------- Analyse bg components ----------------------------
    if (!EvtFullRecon(fTracks, nTracksTT, TTindices, fMCEvent)) {
        if (nTracksTT==2){
            Double_t mass = GetMass(fTracks, nTracksTT, TTindices, fMCEvent);
            if (mass!=-1.) fInvMass_FD->Fill(mass);

            // //////////////////////////////////////////////////////////////////////
            // here check if gamma has made hit in emcal entered 
            Int_t nClusters = fESD->GetNumberOfCaloClusters();
            Double_t dPhiEtaMin;
            Bool_t hasClusterFromGamma = kFALSE;
            for (Int_t ii(0); ii<nClusters; ii++)
            {
                AliESDCaloCluster *clust = fESD->GetCaloCluster(ii);
                if (!clust->IsEMCAL()) continue;
                // if matchTracks is true if at least one track can be propergated to emcal
                // then if the distance betw. the track and the cluster is above a cut value (0.6)
                // we assume that the cluster originates from a gamma 
                if (MatchTracks(clust, fTracks, nTracksTT, TTindices, fMCEvent, dPhiEtaMin)) {
                    if (dPhiEtaMin<0.6) continue;
                }
                // here we assume that the cluster originates from a gamma:
                hasClusterFromGamma = kTRUE;
                // need only one cluster to classify event as bg
                break;
            }
            if (hasClusterFromGamma) fInvMass_GammaDet->Fill(mass);
            // //////////////////////////////////////////////////////////////////////
        } else {
            std::vector<Double_t> mass_vec = 
                GetMassPermute(fTracks,nTracksTT,TTindices,fMCEvent);
            for (Double_t mass : mass_vec) fInvMass_3plusTrks->Fill(mass);
            // now specialize on nb of tracks
            if (nTracksTT==3) for (Double_t mass : mass_vec) fInvMass_3trks->Fill(mass);
        }
    } 
    //////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////
    // ---------------------- Print the event stack ----------------------------------
    /* PrintStack(fMCEvent, kFALSE); */
    // ---------------------- Print the AliESDtracks ---------------------------------
    /* PrintTracks(fTracks, nTracksTT, TTindices); */
    /* printf("----------------------------- evt end ---------------------------------\n\n"); */
    //////////////////////////////////////////////////////////////////////////////////
    
    /* printf("\n\n\n\n----------------------------------------------------\n\n"); */
    PostData(1, fOutList);          // stream the results the analysis of this event to
                                    // the output manager which will take care of writing
                                    // it to a file
}
//_____________________________________________________________________________
void AliAnalysisTaskBG::Terminate(Option_t *)
{
    // terminate
    // called at the END of the analysis (when all events are processed)
    fEvtStorge.PrintNEvts();
}

