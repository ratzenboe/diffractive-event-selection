/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. */
/* See cxx source for full Copyright notice */
/* $Id$ */

#ifndef AliAnalysisTaskEMCAL_H
#define AliAnalysisTaskEMCAL_H

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
#include "CEPBGBase.h"

#include "AliAnalysisTaskSE.h"

class AliAnalysisTaskEMCAL : public AliAnalysisTaskSE, public CEPBGBase
{
    public:
                                AliAnalysisTaskEMCAL();
                                AliAnalysisTaskEMCAL(const char *name,
                                                      Long_t state,
                                                      UInt_t TTmask,
                                                      UInt_t TTpattern,
                                                      TString hitFileName);
        virtual                 ~AliAnalysisTaskEMCAL();

        // these functions also exist in AliAnalysisTaskSE (SE=single event)
        // as we want to change the functions a litte, but dont want to change the 
        // original ones, we make them 'virtual' functions
        virtual void            UserCreateOutputObjects();
        virtual void            UserExec(Option_t* option);
        virtual void            Terminate(Option_t* option);

    private:
        // Output objects 
        TList*                  fOutList;               //! output list
        TH1F*                   fGammaE;                //! energies of primarily produced gammas 
        TH1F*                   fEMCalSecondaryE_SIG;   //! energy of parts reaching the emcal SIG
        TH1F*                   fEMCalSecondaryE_BG;    //! energy of parts reaching the emcal BG
        /* TH1F*                   fGammaE_secondary;   //! energies of gammas in emcal */ 
        TH1F*                   fNeutralPDG;            //! neutral particles pdg
        // emcal hists
        TH1F*                   fEMCnClus_SIG;          //! calo: #clusters in emcal signal
        TH1F*                   fEMCnClus_BG;           //! calo: #clusters in emcal bg

        TH1F*                   fEMCnMatchedClus_SIG;   //! calo: # matched clusters in emcal sig
        TH1F*                   fEMCnMatchedClus_BG;    //! calo: # matched clusters in emcal bg

        TH2F*                   fEMC_nClusVSnMatched_SIG; //! calo: nclus VS n-matched-clus sig
        TH2F*                   fEMC_nClusVSnMatched_BG;  //! calo: nclus VS n-matched-clus bg

        TH1F*                   fEMCenergy_SIG;         //! calo: energy sig
        TH1F*                   fEMCenergy_BG;          //! calo: energy bg

        TH2F*                   fEMC_nClusVSenergy_SIG;  //! calo: 2D #clus vs energy signal
        TH2F*                   fEMC_nClusVSenergy_BG;   //! calo: 2D #clus vs energy bg

        TH1F*                   fEMCal_dphiEta_pion;  //! phi-eta distance of cluster hit to track
        TH1F*                   fEMCal_dphiEta_gamma; //! phi-eta distance of cluster hit to track
        
        // not implemented but neccessary
        AliAnalysisTaskEMCAL(const AliAnalysisTaskEMCAL&); 
        AliAnalysisTaskEMCAL& operator=(const AliAnalysisTaskEMCAL&); 

        // filling the histograms
        void                    EMCalAnalysis(Bool_t isSignal, AliMCEvent* MCevt, 
                                        TObjArray* tracks, Int_t nTracksTT, TArrayI* TTindices);

        void                    EMCalHits(Bool_t isSignal);

        ClassDef(AliAnalysisTaskEMCAL, 1);
};

#endif
