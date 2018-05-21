/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. */
/* See cxx source for full Copyright notice */
/* $Id$ */

#ifndef AliAnalysisTaskMCInfo_H
#define AliAnalysisTaskMCInfo_H

#include "AliAnalysisTaskSE.h"

class AliAnalysisTaskMCInfo : public AliAnalysisTaskSE  
{
    public:
                                AliAnalysisTaskMCInfo();
                                AliAnalysisTaskMCInfo(const char *name,
                                                      Long_t state,
                                                      UInt_t TTmask,
                                                      UInt_t TTpattern);
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
        Long_t                  fAnalysisStatus;    //  stores the analysis-status 
        UInt_t                  fTTmask;            //  track conditions
        UInt_t                  fTTpattern;         //  track conditions
        // Output objects 
        TList*                  fOutList;           //! output list
        TH1F*                   fGammaE;            //! energies of primarily produced gammas 
        /* TH1F*                   fSecondaryE_SIG; //! energies of primarily produced gammas */ 
        /* TH1F*                   fSecondaryE_BG;  //! energies of primarily produced gammas */ 
        /* TH1F*                   fGammaE_secondary;  //! energies of gammas in emcal */ 
        TH1F*                   fNeutralPDG;        //! neutral particles pdg
        // emcal hists
        TH1F*                   fDistBadChannel_SIG; //! calo: distance to bad channel signal
        TH1F*                   fDistBadChannel_BG;  //! calo: distance to bad channel bg
        TH1F*                   fEMCnClus_SIG;       //! calo: #clusters in emcal signal
        TH1F*                   fEMCnClus_BG;        //! calo: #clusters in emcal bg

        TH1F*                   fEMCnMatchedClus_SIG; //! calo: # matched clusters in emcal sig
        TH1F*                   fEMCnMatchedClus_BG; //! calo: # matched clusters in emcal bg
        TH2F*                   fEMC_nClusVSnMatched_SIG; //! calo: nclus VS n-matched-clus sig
        TH2F*                   fEMC_nClusVSnMatched_BG; //! calo: nclus VS n-matched-clus bg

        TH1F*                   fEMCDphi_SIG;            //! distance to closest track in phi sig
        TH1F*                   fEMCDphi_BG;            //! distance to closest track in phi bg
        TH1F*                   fEMCDz_SIG;              //! distance to closest track in z sig
        TH1F*                   fEMCDz_BG;              //! distance to closest track in z bg

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
        TH1F*                   fEmcalHitMothers_SIG;   //! mothers of the hitting particles (sig)
        TH1F*                   fEmcalHitMothers_BG;   //! mothers of the hitting particles (bg)

        TH1F*                   fEMCal_dphiEta_SIG;  //! phi-eta distance of cluster hit to track
        TH1F*                   fEMCal_dphiEta_BG;   //! phi-eta distance of cluster hit to track
        
        // not implemented but neccessary
        AliAnalysisTaskMCInfo(const AliAnalysisTaskMCInfo&); 
        AliAnalysisTaskMCInfo& operator=(const AliAnalysisTaskMCInfo&); 

        Bool_t IsSTGFired(TBits* fFOmap,Int_t dphiMin=0,Int_t dphiMax=10);
        TLorentzVector GetXLorentzVector(AliMCEvent* MCevent);
        void PrintStack(AliMCEvent* MCevent, Bool_t prim=kTRUE);
        void EMCalAnalysis(Bool_t isSignal, Int_t nTracksTT, TArrayI* TTindices);
        Bool_t MatchTracks(AliESDCaloCluster* clust, Int_t nTracksTT, TArrayI* TTindices, 
                           Double_t& dPhiEtaMin);
        Bool_t IsClusterFromPDG(AliESDCaloCluster* clust, Int_t pdg);
        ClassDef(AliAnalysisTaskMCInfo, 1);
};

#endif
