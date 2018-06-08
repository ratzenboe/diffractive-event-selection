#ifndef EventDef_H
#define EventDef_H

#include <vector>
#include <map>

#include <TString.h>

#pragma link C++ class std::map<Int_t, std::string>;

class EventDef
{
  public:
                                    EventDef(Int_t rootpdg=9900110, TString rootstr="X");
                                    ~EventDef();

    // Structure holding particle-information
    struct                          Particle {
        Int_t Number; 
        Int_t Pdg; 
        Int_t MotherNumber; 
        std::vector<Int_t> DaughterVec;
        Bool_t isFinal; 
        // equality comparison
        bool operator==(const Particle& a) const
        {
            return (Pdg==a.Pdg && MotherNumber==a.MotherNumber && isFinal==a.isFinal);
        }
    } ;

    // Modifiers
    void                            Reset();
    // add track
    void                            AddTrack(Int_t number, Int_t pdg, 
                                             Int_t mother_number,
                                             Int_t daugther_nb_1, Int_t daugther_nb_2, 
                                             Bool_t isFinal);
    
    // Finalize event: sort particles, order daugthers rewrite fParticles
    // The class only displays the event in a compact way; 
    // the comparison is only correctly done after this function is called
    void                            FinalizeEvent();
    
    // Setter
    void                            IncreaseDecayOccurance() { fDecayOccurance++; }

    // Getters
    Int_t                           GetnParticles() const { return fnParticles; }
    Int_t                           GetnUniqueParticles() const { return fParticles.size(); }

    Int_t                           GetOccurance() const { return fDecayOccurance; }
    Int_t                           GetIsFinalized() const { return fIsFinalized; }

    Int_t                           GetTrackPdg(UInt_t i) const;
    Int_t                           GetTrackMotherPdg(UInt_t i) const;
    Int_t                           GetTrackIsFinal(UInt_t i) const;
    Int_t                           GetTrackOccurance(UInt_t i) const;

    TString                         GetDecayStringShort() const;
    TString                         GetDecayStringLong() const;

    void                            PrintEvent(TString filename="decaymodes.tex") const;

    // check the final states
    Bool_t                          IsEMCALCase() const;
    Bool_t                          IsThreePlusCase() const;
    
    // Comparison operator (check if two events are the same)
    Bool_t                          operator == (const EventDef& other) const;


  private:
    std::vector<Particle>           fParticles; 
    Int_t                           fnParticles;
    std::map<Int_t, std::string>    fParticleCodes;
    Int_t                           fDecayOccurance;
    Bool_t                          fIsFinalized;
    Int_t                           fRootPDG;
    std::string                     fRootString;

    void                            SetParticleCodes();
    // Get certain track
    Int_t                           GetParticleIndexFromNumber(Int_t number) const;
    // recursive function looping over the decay tree formulating the 
    Int_t                           TreeLooper(Int_t mother, TString& decaystring) const;

    // check if all daughters are final
    Bool_t                          AllDaughtersFinal(Int_t mother_number) const;
    // particle sorting, neccessary for event comparison
    void                            SortParticles();
    // order the daugther particles in an acending pdg-code order (make each decay-string equal)
    Int_t                           OrderDaugthers(Int_t mother=1);

    // check if the particle is directely from the X particle
    Bool_t                          IsFromX(Int_t number) const;



    ClassDef(EventDef, 1)
};

#endif
