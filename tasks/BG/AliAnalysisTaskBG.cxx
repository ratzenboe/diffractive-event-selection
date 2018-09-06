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
#include "TSystem.h"
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
  // nb of events info
  , fAllEvts(0)
  , fnEvtsPassedFilter(0)
  , fnEvtsTotal(0)
  , fFilesList("")
  // output objects
  , fOutList(0)
  , fInvMass_Sig(0)
  , fInvMass_FD(0)
  , fInvMass_FD_emcal(0)
  , fInvMass_FD_3plus(0)
  , fInvMass_FD_other(0)
  , fInvMass_FD_hasGammas(0)
  // BG 3+ study
  , fInvMass_3trks(0)
  , fInvMass_4trks(0)
  , fInvMass_5trks(0)
  , fInvMass_6trks(0)
  , fInvMass_3plusTrks(0)
  // Gamma BG study
  , fInvMass_GammaDet_bg(0)
  , fInvMass_GammaDet_sig(0)
  // Likesign histograms
  , fInvMass_LS_plus(0)
  , fInvMass_LS_minus(0)
  , fNb_trks_passed(0)
  // ivmass 2pi, no emc-cluster
  , fInvMass_noBGCluster_sig(0)
  , fInvMass_noBGCluster_FD(0)
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
  // nb of events info
  , fAllEvts(0)
  , fnEvtsPassedFilter(0)
  , fnEvtsTotal(0)
  , fFilesList("")
  // output objects
  , fOutList(0)
  , fInvMass_Sig(0)
  , fInvMass_FD(0)
  , fInvMass_FD_emcal(0)
  , fInvMass_FD_3plus(0)
  , fInvMass_FD_other(0)
  , fInvMass_FD_hasGammas(0)
  // BG 3+ study
  , fInvMass_3trks(0)
  , fInvMass_4trks(0)
  , fInvMass_5trks(0)
  , fInvMass_6trks(0)
  , fInvMass_3plusTrks(0)
  // Gamma BG study
  , fInvMass_GammaDet_bg(0)
  , fInvMass_GammaDet_sig(0)
  // Likesign histograms
  , fInvMass_LS_plus(0)
  , fInvMass_LS_minus(0)
  , fNb_trks_passed(0)
  // ivmass 2pi, no emc-cluster
  , fInvMass_noBGCluster_sig(0)
  , fInvMass_noBGCluster_FD(0)
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
    fInvMass_Sig = new TH1F("fInvMass_Sig", "fInvMass_Sig",100, 0, 3);

    fInvMass_FD = new TH1F("fInvMass_FD", "fInvMass_FD",100, 0, 3);
    fInvMass_FD_emcal = new TH1F("fInvMass_FD_emcal", "fInvMass_FD_emcal",100, 0, 3);
    fInvMass_FD_3plus = new TH1F("fInvMass_FD_3plus", "fInvMass_FD_3plus",100, 0, 3);
    fInvMass_FD_other = new TH1F("fInvMass_FD_other", "fInvMass_FD_other",100, 0, 3);
    fInvMass_FD_hasGammas = new TH1F("fInvMass_FD_hasGammas", "fInvMass_FD_hasGammas",100, 0, 3);
    // BG 3+
    fInvMass_3trks = new TH1F("fInvMass_3trks", "fInvMass_3trks",100, 0, 3);
    fInvMass_4trks = new TH1F("fInvMass_4trks", "fInvMass_4trks",100, 0, 3);
    fInvMass_5trks = new TH1F("fInvMass_5trks", "fInvMass_5trks",100, 0, 3);
    fInvMass_6trks = new TH1F("fInvMass_6trks", "fInvMass_6trks",100, 0, 3);
    fInvMass_3plusTrks = new TH1F("fInvMass_3plusTrks", "fInvMass_3plusTrks",100, 0, 3);
    // BG gamma hit
    fInvMass_GammaDet_sig = new TH1F("fInvMass_GammaDet_sig", "fInvMass_GammaDet_sig",100, 0, 3);
    fInvMass_GammaDet_bg = new TH1F("fInvMass_GammaDet_bg", "fInvMass_GammaDet_bg",100, 0, 3);
    // Likesign histograms
    fInvMass_LS_plus = new TH1F("fInvMass_LS_plus", "fInvMass_LS_plus",100, 0, 3);
    fInvMass_LS_minus = new TH1F("fInvMass_LS_minus", "fInvMass_LS_minus",100, 0, 3);

    fNb_trks_passed = new TH1I("fNb_trks_passed", "fNb_trks_passed", 8, 2, 10);

    fInvMass_noBGCluster_sig = new TH1F("fInvMass_noBGCluster_sig", "fInvMass_noBGCluster_sig",100, 0, 3);
    fInvMass_noBGCluster_FD = new TH1F("fInvMass_noBGCluster_FD", "fInvMass_noBGCluster_FD",100, 0, 3);

    fOutList->Add(fInvMass_Sig);          
    fOutList->Add(fInvMass_FD);          
    fOutList->Add(fInvMass_FD_emcal);          
    fOutList->Add(fInvMass_FD_3plus);          
    fOutList->Add(fInvMass_FD_other);          
    fOutList->Add(fInvMass_FD_hasGammas);          
    fOutList->Add(fInvMass_3trks);          
    fOutList->Add(fInvMass_4trks);          
    fOutList->Add(fInvMass_5trks);          
    fOutList->Add(fInvMass_6trks);          
    fOutList->Add(fInvMass_3plusTrks);          
    fOutList->Add(fInvMass_GammaDet_bg);          
    fOutList->Add(fInvMass_GammaDet_sig);          
    fOutList->Add(fInvMass_LS_plus);          
    fOutList->Add(fInvMass_LS_minus);          
    fOutList->Add(fNb_trks_passed);          
    fOutList->Add(fInvMass_noBGCluster_sig);          
    fOutList->Add(fInvMass_noBGCluster_FD);          

    PostData(1, fOutList);              // postdata will notify the analysis manager of changes 
                                        // and updates to the fOutList object. 
                                        // the manager will in the end take care of writing 
                                        // the output to file so it needs to know what's 
                                        // in the output
}

//_____________________________________________________________________________
void AliAnalysisTaskBG::UserExec(Option_t *)
{
    fAllEvts++;
    // appned the current directory to the final string->to check if loop over all files
    TString path_str = GetDirFromFullPath(CurrentFileName());
    if (!fFilesList.EndsWith(path_str)) fFilesList += "\n" + path_str;
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
    Int_t nTracksTT;
    std::vector<Int_t> nTracksAccept = { 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    TArrayI *TTindices  = new TArrayI();
    Bool_t isGoodEvt = lhc16filter(fESD,nTracksAccept,fTTmask,fTTpattern,nTracksTT,TTindices); 
    if (!isGoodEvt) return ;
    
    // fill signal information
    std::vector<Int_t> good_evt_particles = {211, -211};
    if (HasRightParticles(fTracks, nTracksTT, TTindices, 
                           fPIDResponse, fPIDCombined, 
                           fMCEvent, good_evt_particles)) {
        // has pi+ pi-
        if (EvtFullRecon(fTracks, nTracksTT, TTindices, fMCEvent)) {
            fInvMass_Sig->Fill(GetMass(fTracks, nTracksTT, TTindices, fMCEvent));
        }
    }

    fnEvtsPassedFilter++;
    std::vector<Int_t> ls_plus_evt = {211, 211};
    std::vector<Int_t> ls_minus_evt = {-211, -211};
    if (HasRightParticles(fTracks, nTracksTT, TTindices, 
                          fPIDResponse, fPIDCombined,
                          fMCEvent, ls_plus_evt)) {
        fInvMass_LS_plus->Fill(GetMass(fTracks, nTracksTT, TTindices, fMCEvent)); 
        return ;
    }
    if (HasRightParticles(fTracks, nTracksTT, TTindices, 
                          fPIDResponse, fPIDCombined,
                          fMCEvent, ls_minus_evt)) {
        fInvMass_LS_minus->Fill(GetMass(fTracks, nTracksTT, TTindices, fMCEvent)); 
        return ;
    }
    // we look for pion evts
    Int_t pdg_pass = 211;
    if (!HasRightParticles(fTracks, nTracksTT, TTindices, 
                           fPIDResponse, fPIDCombined, 
                           fMCEvent, pdg_pass)) return ;
    fNb_trks_passed->Fill(nTracksTT);
    // ///////////////////////////////////////////////////////////////////////////////
    // ---------------------------- Analyse type of decay ----------------------------
    /* if(nTracksTT==2) NewEvent(fMCEvent); */
    //////////////////////////////////////////////////////////////////////////////////
    if (nTracksTT!=TTindices->GetSize()) {
        printf("\n\n\n nTracksTT: %i != TTindices->GetSize(): %i\n", 
                nTracksTT, TTindices->GetSize());
        return ;
    }    
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
        if (MatchTracks(clust, fTracks, nTracksTT, TTindices, dPhiEtaMin)) {
            if (dPhiEtaMin<=0.51) continue;
        }
        // here we assume that the cluster originates from a gamma:
        hasClusterFromGamma = kTRUE;
        // need only one cluster to classify event as bg
        break;
    }
    Bool_t isEvtFullRecon = EvtFullRecon(fTracks, nTracksTT, TTindices, fMCEvent);
    if (isEvtFullRecon && nTracksTT==2) {
        // emcal hit invmass contribution from sig evts
        Double_t mass_fullr = GetMass(fTracks, nTracksTT, TTindices, fMCEvent);
        if (mass_fullr==-1) return ;
        if (hasClusterFromGamma) {
            fInvMass_GammaDet_sig->Fill(mass_fullr);
        } else fInvMass_noBGCluster_sig->Fill(mass_fullr);
    } 
    //////////////////////////////////////////////////////////////////////////////////
 
    //////////////////////////////////////////////////////////////////////////////////
    // ---------------------------- Analyse bg components ----------------------------
    if (!isEvtFullRecon) {
        if (nTracksTT==2) {
            Double_t mass = GetMass(fTracks, nTracksTT, TTindices, fMCEvent);
            if (mass!=-1.) fInvMass_FD->Fill(mass);
            else return ;
            // read description of fInvMass_FD_hasGammas in header file
            if (HasGammas(fMCEvent)) fInvMass_FD_hasGammas->Fill(mass);
            //////////////////////////////////////////////////////////////////////////
            // ---------------- bg detectable study ----------------------------------
            // distribute the fd-mass to the corresponding BG event
            if (OnlyGammas(fMCEvent)) fInvMass_FD_emcal->Fill(mass);
            else if (ThreePlusCase(fMCEvent)) fInvMass_FD_3plus->Fill(mass);
            else fInvMass_FD_other->Fill(mass);
            //////////////////////////////////////////////////////////////////////////
            // ------------------ emcal real fd bg -----------------------------------
            if (hasClusterFromGamma) fInvMass_GammaDet_bg->Fill(mass);
            else fInvMass_noBGCluster_FD->Fill(mass);
            // //////////////////////////////////////////////////////////////////////
        } else {
            /////////////////////////////////////////////////////////////////////////
            // --------------------------- 3+ bg ------------------------------------
            std::vector<Double_t> mass_vec = 
                GetMassPermute(fTracks,nTracksTT,TTindices,fMCEvent);
            for (Double_t mass : mass_vec) fInvMass_3plusTrks->Fill(mass);
            // now specialize on nb of tracks
            if (nTracksTT==3) for (Double_t mass : mass_vec) fInvMass_3trks->Fill(mass);
            if (nTracksTT==4) for (Double_t mass : mass_vec) fInvMass_4trks->Fill(mass);
            if (nTracksTT==5) for (Double_t mass : mass_vec) fInvMass_5trks->Fill(mass);
            if (nTracksTT==6) for (Double_t mass : mass_vec) fInvMass_6trks->Fill(mass);
            /////////////////////////////////////////////////////////////////////////
        }
    } 
    //////////////////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////////////////////
    // ---------------------- Print the event stack ----------------------------------
    /* PrintStack(fMCEvent, kTRUE); */
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
    /* fEvtStorge.PrintNEvts(); */
    // print number of passed evts
    /* WriteToFile(fFilesList, "fileslist.txt"); */ 
    printf("%i/%i events are usable\n", fnEvtsTotal, fAllEvts);
    printf("%i/%i passed lhc16filter\n", fnEvtsPassedFilter, fnEvtsTotal);
}

