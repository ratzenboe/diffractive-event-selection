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
        TH1F*                   fGammaE;            //! energies of gammas in emcal
        TH1F*                   fNeutralPDG;        //! neutral particles pdg
        TH1F*                   fEmcalHitMothers;   //! mothers of the hitting particles 
        // not implemented but neccessary
        AliAnalysisTaskMCInfo(const AliAnalysisTaskMCInfo&); 
        AliAnalysisTaskMCInfo& operator=(const AliAnalysisTaskMCInfo&); 

        Bool_t IsSTGFired(TBits* fFOmap,Int_t dphiMin=0,Int_t dphiMax=10);
        TLorentzVector GetXLorentzVector(AliMCEvent* MCevent);

        ClassDef(AliAnalysisTaskMCInfo, 1);
};

#endif
