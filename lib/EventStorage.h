#ifndef EventStorage_H
#define EventStorage_H

#include <vector>
#include <TString.h>

#include "EventDef.h"

class EventStorage 
{
  public:
                                    EventStorage();
                                    ~EventStorage();

    // Modifiers
    void                            Reset();
    // Add Event
    void                            AddEvent(EventDef event);

    // Getters
    Int_t                           GetNEvents() const { return fnEvents; }
    Int_t                           GetNUniqueEvents() const { return fEvents.size(); }

    void                            PrintNEvts(TString filename="decaymodes.tex",Int_t nb=-1) const;

  private:
    std::vector<EventDef>           fEvents; 
    Int_t                           fnEvents;

    // sort events by increasing number of occurance
    void                            SortEvents();

    ClassDef(EventStorage, 1)
};

#endif
