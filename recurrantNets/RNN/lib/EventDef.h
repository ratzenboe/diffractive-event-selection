#ifndef EventDef_H
#define EventDef_H

#include <vector>
#include <map>

#include <TString.h>

#pragma link C++ class std::map<Int_t, std::string>;

class EventDef
{
  public:
                                    EventDef();
                                    ~EventDef();

    // Structure holding particle-information
    struct                          Particle {
        Int_t Number; 
        Int_t Pdg; 
        Int_t MotherNumber; 
        Int_t DaughterNbs[2]; 
        Bool_t isFinal; 
    } ;

    // Modifiers
    void                            Reset();
    // add track
    void                            AddTrack(Int_t number, Int_t pdg, 
                                             Int_t mother_number,
                                             Int_t daugther_nb_1, Int_t daugther_nb_2, 
                                             Bool_t isFinal);
    
    // Getters
    Int_t                           GetnParticles() const { return fnParticles; }
    Int_t                           GetnUniqueParticles() const { return fParticles.size(); }
    Int_t                           GetnParticlesFromX() const { return fnParticlesFromX; }

    Int_t                           GetTrackPdg(UInt_t i) const;
    Int_t                           GetTrackMotherPdg(UInt_t i) const;
    Int_t                           GetTrackIsFinal(UInt_t i) const;

    TString                         GetDecayStringShort() const;
    TString                         GetDecayStringLong() const;

    void                            PrintEvent(TString filename="decayfile.txt") const;
    
    // Comparison operator (check if two events are the same)
    Bool_t                          operator == (const EventDef& other) const;

  private:
    std::vector<Particle>           fParticles; 
    Int_t                           fnParticles;
    Int_t                           fnParticlesFromX;
    std::map<Int_t, std::string>    fParticleCodes;

    void                            SortParticles();
    void                            SetParticleCodes();

    Int_t                           TreeLooper(Int_t mother);
    // Get certain track
    Int_t                           GetTrackIndexInVector(Int_t number) const;

    ClassDef(EventDef, 1)
};

#endif
