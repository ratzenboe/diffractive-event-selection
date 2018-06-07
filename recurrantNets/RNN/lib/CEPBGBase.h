#ifndef CEPBGBase_H
#define CEPBGBase_H

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
#include "AliPIDResponse.h"
#include "AliPIDCombined.h"

#include "EventStorage.h"

class CEPBGBase
{
    public:
                                CEPBGBase();
                                CEPBGBase(Long_t state,
                                          UInt_t TTmask,
                                          UInt_t TTpattern,
                                          TString hitFileName);
        virtual                 ~CEPBGBase();
        

    protected:
        AliESDEvent*            fESD;               //! input event
        AliTriggerAnalysis*     fTrigger;           //! trigger object
        TArrayI*                fTrackStatus;       //! array of track-status
        TObjArray*              fTracks;            //! array of AliESDtracks
        AliCEPUtils*            fCEPUtil;           //! AliCEPUtil object
        AliPIDResponse*         fPIDResponse;       //! pid response
        AliPIDCombined*         fPIDCombined;       //! bayes pid 
        // hit file specific variables
        TClonesArray*           fHitsArray;         //! array containing hits
        TString                 fHitFileName;       //  filename of hit file
        TFile*                  fHitFile;           //! hit file
        TTree*                  fHitTree;           //! tree containing hits
        TBranch*                fHitBranch;         //! corresponding branch
        TDirectory*             fHitDir;            //! hits are stored event-wise in directories
        TString                 fCurrentDir;        //  current ESD-working directory
        // Class storing the event for printing 
        EventStorage            fEvtStorge;         //! event printing class
 
        Long_t                  fAnalysisStatus;    //  stores the analysis-status 
        UInt_t                  fTTmask;            //  track conditions
        UInt_t                  fTTpattern;         //  track conditions
       
        // not implemented but neccessary
        CEPBGBase(const CEPBGBase&); 
        CEPBGBase& operator=(const CEPBGBase&); 

        // initializing important member variables -> has to be called BEFORE UserExec() function
        void                    Initialize();

        // prefiltering 
        Bool_t                  lhc16filter(AliESDEvent* esd_evt, Int_t nTracksAccept, 
                                            UInt_t TTmask, UInt_t TTpattern,
                                            Int_t& nTracksTT, TArrayI*& TTindices) const;
        Bool_t                  lhc16filter(AliESDEvent* esd_evt, std::vector<Int_t> nTrksAcc_vec, 
                                            UInt_t TTmask, UInt_t TTpattern,
                                            Int_t& nTracksTT, TArrayI*& TTindices) const;
        // heart of the lhc16-filter
        Bool_t                  EventFilter(AliESDEvent* esd_evt, Int_t nTracksAccept) const;
        // part of the lhc16filter
        Bool_t                  IsSTGFired(TBits* fFOmap,Int_t dphiMin=0,Int_t dphiMax=10) const;
        // Selection of pions
        Bool_t                  IsPionEvt(TObjArray* tracks, Int_t nTracksTT, 
                                          TArrayI* TTindices, 
                                          AliPIDResponse* pidResponse, 
                                          AliPIDCombined* pidCombined,
                                          AliMCEvent* MCevt=0x0) const;

        // event characterization functions
        // check if event is fully reconstructed
        Bool_t                  EvtFullRecon(TObjArray* tracks, Int_t nTracksTT, 
                                             TArrayI* TTindices, AliMCEvent* MCevt) const;
        TLorentzVector          GetXLorentzVector(AliMCEvent* MCevent) const;
        // get lorentz vector of particle index
        TParticle*              GetPartByLabel(Int_t MCind, AliMCEvent* MCevt) const;
        // get mass of event
        Double_t                GetMass(TObjArray* tracks, Int_t nTracksTT, 
                                        TArrayI* TTindices, AliMCEvent* MCevt) const;
        std::vector<Double_t>   GetMassPermute(TObjArray* tracks, Int_t nTracksTT, 
                                               TArrayI* TTindices, AliMCEvent* MCevt) const;

        // emcal hit file related functions:
        // Updating path to hit file:
        //  -> in UserExec called like: UpdateGlobalVars(CurrentFilName(), Entry());
        //  CurrentFileName() & Entry() functions of AliAnalysisTaskSE
        Bool_t                  UpdateGlobalVars(const char* currentFileName, Int_t entry);
        // retrieve directory path from path to file in said directory
        TString                 GetDirFromFullPath(const char* fullpath);
        // check if cluster stems from particle with certain pdg value
        Bool_t                  IsClusterFromPDG(AliESDCaloCluster* clust, Int_t& pdg,
                                                 AliMCEvent* MCevt);
        // Match cluster to track & if true passed by reference var dPhiEtaMin is describing dist
        Bool_t                  MatchTracks(AliESDCaloCluster* clust, 
                                            TObjArray* tracks, Int_t nTracksTT, TArrayI* TTindices, 
                                            AliMCEvent* MCevt, Double_t& dPhiEtaMin);

        // fill EventStorage with new event
        void                    NewEvent(AliMCEvent* MCevt);
 

        //////////////////////////////////////////////////////////////////////////////////
        // ------------------------------ Print functions --------------------------------
        // print particle stack
        void                    PrintStack(AliMCEvent* MCevent, Bool_t prim=kTRUE) const;
        // print particle stack
        void                    PrintTracks(TObjArray* tracks, Int_t nTracksTT, 
                                            TArrayI* TTindices) const;
        // print emcal hits
        void                    PrintEMCALHits() const;
        //////////////////////////////////////////////////////////////////////////////////

        ClassDef(CEPBGBase, 1);
};

#endif
