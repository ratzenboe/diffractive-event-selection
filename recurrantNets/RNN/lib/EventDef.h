#ifndef EventDef_H
#define EventDef_H

#include <vector>

class EventDef
{
  public:
                                EventDef();
                                ~EventDef();

    // Structure holding particle-information
    struct                      Particle {
        Int_t Pdg; 
        Int_t MotherPdg; 
        Int_t Occurance; 
    } ;

    // add track
    void                        AddTrack(Int_t pdg, Int_t mother);
    
    // Getters
    Int_t                       nParticles() const { return fParticles.size(); }
    Int_t                       TrackPdg(UInt_t i) const;
    Int_t                       TrackOccurance(UInt_t i) const;
    Int_t                       TrackMotherPdg(UInt_t i) const;
    // Modifiers
    void                        Reset();
    void                        SortParticles();


    Bool_t                      operator == (const EventDef& other) const;

  private:
    std::vector<Particle>       fParticles; 

    ClassDef(EventDef, 1)
};

#endif
