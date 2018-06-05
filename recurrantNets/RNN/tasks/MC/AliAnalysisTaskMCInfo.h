/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. */
/* See cxx source for full Copyright notice */
/* $Id$ */

#ifndef AliAnalysisTaskMCInfo_H
#define AliAnalysisTaskMCInfo_H

#include "TBits.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TList.h"
#include "TBranch.h"
#include "TDirectory.h"
#include "TClonesArray.h"
#include "TString.h"
#include "TList.h"
#include "TArrayI.h"
#include "TObjArray.h"
#include "AliESDEvent.h"
#include "AliMCEvent.h"
#include "AliESDCaloCluster.h"
#include "AliTriggerAnalysis.h"
#include "AliCEPUtils.h"
#include "AliEMCALGeometry.h"

#include "EventStorage.h"
#include "EventDef.h"

#include "AliAnalysisTaskSE.h"

class AliAnalysisTaskMCInfo : public AliAnalysisTaskSE  
{
    public:
                                AliAnalysisTaskMCInfo();
                                AliAnalysisTaskMCInfo(const char *name,
                                                      Long_t state,
                                                      UInt_t TTmask,
                                                      UInt_t TTpattern,
                                                      TString hitFileName);
        virtual                 ~AliAnalysisTaskMCInfo();

        // these functions also exist in AliAnalysisTaskSE (SE=single event)
        // as we want to change the functions a litte, but dont want to change the 
        // original ones, we make them 'virtual' functions
        virtual void            UserCreateOutputObjects();
        virtual void            UserExec(Option_t* option);
        virtual void            Terminate(Option_t* option);

    private:
        AliESDEvent*            fESD;               //! input event
        AliTriggerAnalysis*     fTrigger;           //! trigger object
        TArrayI*                fTrackStatus;       //! array of track-status
        TObjArray*              fTracks;            //! array of AliESDtracks
        AliCEPUtils*            fCEPUtil;           //! AliCEPUtil object

        TString                 fHitFileName;       //  EMCal hit file 
        TFile*                  fHitFile;           //! EMCal hit file 
        TTree*                  fHitTree;           //! EMCal hit tree 
        TBranch*                fHitBranch;         //! EMCal hit branch 
        TDirectory*             fHitDir;            //! EMCal hit directory 
        TClonesArray*           fHitsArray;         //! EMCal hit clones array
        TString                 fCurrentDir;        //  current ESD-working directory 

        Long_t                  fAnalysisStatus;    //  stores the analysis-status 
        UInt_t                  fTTmask;            //  track conditions
        UInt_t                  fTTpattern;         //  track conditions
        // Output objects 
        TList*                  fOutList;           //! output list
        TH1F*                   fGammaE;            //! energies of primarily produced gammas 
        TH1F*                   fEMCalSecondaryE_SIG; //! energy of parts reaching the emcal SIG
        TH1F*                   fEMCalSecondaryE_BG;  //! energy of parts reaching the emcal BG
        /* TH1F*                   fGammaE_secondary;  //! energies of gammas in emcal */ 
        TH1F*                   fNeutralPDG;        //! neutral particles pdg
        // emcal hists
        TH1F*                   fDistBadChannel_SIG; //! calo: distance to bad channel signal
        TH1F*                   fDistBadChannel_BG;  //! calo: distance to bad channel bg
        TH1F*                   fEMCnClus_SIG;       //! calo: #clusters in emcal signal
        TH1F*                   fEMCnClus_BG;        //! calo: #clusters in emcal bg

        TH1F*                   fEMCnMatchedClus_SIG;  //! calo: # matched clusters in emcal sig
        TH1F*                   fEMCnMatchedClus_BG;   //! calo: # matched clusters in emcal bg
        TH2F*                   fEMC_nClusVSnMatched_SIG; //! calo: nclus VS n-matched-clus sig
        TH2F*                   fEMC_nClusVSnMatched_BG;  //! calo: nclus VS n-matched-clus bg

        TH1F*                   fEMCDphi_SIG;          //! distance to closest track in phi sig
        TH1F*                   fEMCDphi_BG;           //! distance to closest track in phi bg
        TH1F*                   fEMCDz_SIG;            //! distance to closest track in z sig
        TH1F*                   fEMCDz_BG;             //! distance to closest track in z bg

        TH1F*                   fPHOSnClus_SIG;      //! calo: #clusters in phos signal
        TH1F*                   fPHOSnClus_BG;       //! calo: #clusters in phos bg
        TH1F*                   fEMCenergy_SIG;      //! calo: energy sig
        TH1F*                   fEMCenergy_BG;       //! calo: energy bg
        TH1F*                   fPHOSenergy_SIG;     //! calo: energy sig
        TH1F*                   fPHOSenergy_BG;      //! calo: energy bg

        TH2F*                   fEMC_nClusVSenergy_SIG;  //! calo: 2D #clus vs energy signal
        TH2F*                   fEMC_nClusVSenergy_BG;   //! calo: 2D #clus vs energy bg
        TH2F*                   fPHOS_nClusVSenergy_SIG; //! calo: 2D #clus vs energy signal
        TH2F*                   fPHOS_nClusVSenergy_BG;  //! calo: 2D #clus vs energy bg
        TH1F*                   fEmcalHitMothers_SIG;    //! mothers of the hitting particles (sig)
        TH1F*                   fEmcalHitMothers_BG;     //! mothers of the hitting particles (bg)

        TH1F*                   fEMCal_dphiEta_SIG;  //! phi-eta distance of cluster hit to track
        TH1F*                   fEMCal_dphiEta_BG;   //! phi-eta distance of cluster hit to track
        
        EventStorage            fEvtStorge;         
        // not implemented but neccessary
        AliAnalysisTaskMCInfo(const AliAnalysisTaskMCInfo&); 
        AliAnalysisTaskMCInfo& operator=(const AliAnalysisTaskMCInfo&); 

        // print emcal hits
        void                    PrintEMCALHits(Bool_t isSignal);
        // filling the histograms
        void                    EMCalAnalysis(Bool_t isSignal, Int_t nTracksTT, 
                                              TArrayI* TTindices);
        // match tracks to the emcal
        Bool_t                  MatchTracks(AliESDCaloCluster* clust, Int_t nTracksTT, 
                                            TArrayI* TTindices, Double_t& dPhiEtaMin);
        // check if calo-cluster originates from certain particle (including secondaries)
        Bool_t                  IsClusterFromPDG(AliESDCaloCluster* clust, Int_t& pdg);
        // get directory from full path to AliESDs.root file
        TString                 GetDirFromFullPath(const char* fullpath);
        // update global variables as well as the hit-tree/branch
        Bool_t                  UpdateGlobalVars();

        ClassDef(AliAnalysisTaskMCInfo, 1);
};

#endif
