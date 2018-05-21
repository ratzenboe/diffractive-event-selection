#include <algorithm>

#include <TDirectory.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TClonesArray.h>
#include <TCollection.h>
#include <AliEMCALHit.h>

void ReadEMCALHits(TString hitfile)
{
    TFile* f = TFile::Open(hitfile);
    TTree* hitstree = 0x0;
    TDirectory* dir = 0x0;
    TBranch* hitsbranch = 0x0;
    TString iEvtStr;
    TClonesArray * hitsArray = new TClonesArray("AliEMCALHit",1000);
    std::vector<Int_t> parent_v;
    for (Int_t iev=0; iev<10; iev++) {
        iEvtStr.Form("Event%d", iev);
        dir = f->GetDirectory(iEvtStr);
        dir->GetObject("TreeH", hitstree);
        if (hitstree) {
            hitsbranch = hitstree->GetBranch("EMCAL");
            hitsbranch->SetAddress(&hitsArray);
            if (hitsbranch)
                printf("----------------- Event %i -----------------\n",iev);
                parent_v.clear();
                for (Int_t iHit(0); iHit<hitsbranch->GetEntries(); iHit++){
                    hitsbranch->GetEntry(iHit);
                    TIter next(hitsArray);
                    AliEMCALHit* hit;
                    while( (hit=dynamic_cast<AliEMCALHit*>(next())) )
                    {
                        // if the parent particle has already been counted we continue
                        if(std::find(parent_v.begin(), parent_v.end(), hit->GetIparent()) 
                                != parent_v.end()) continue;

                        printf("Initial energy of parent part: %-3.3f", hit->GetIenergy());
                        printf("  <- Parent Id: %i", hit->GetIparent());
                        printf("  <- GetPrimary(): %i", hit->GetPrimary());
                        printf("  <- Primary entrance E(): %-3.3f\n", hit->GetPe());

                        parent_v.push_back(hit->GetIparent());
                    }
                }
                hitsArray->Clear();
                printf("----------------- Event end -----------------\n");
        } else { printf("TreeH does not exist\n"); }
    }
}
