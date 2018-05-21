void ReadHits(TString hitfile, TString det="EMCAL")
{
  TFile* f = new TFile(hitfile,"READ");
  TTree   * hitstree;
  TBranch * hitsbranch;
  char treeName[20];
  TString clones_array_str;
  clones_array_str = "Ali"+det+"Hit"
  TClonesArray * hits = new TClonesArray(clones_array_hit,1000) ;
  for (Int_t iev=0; iev<10; iev++) {
    sprintf(treeName,"TreeH\%d",iev);
    hitstree = (TTree*)f->Get(treeName);
    if (hitstree) {
      hitsbranch = hitstree->GetBranch(det);
      hitsbranch->SetAddress(&hits);
      hitsbranch->GetEntry(0);
      printf("Hits arrays contains %d entries\n",hits->GetEntries());
      hits->Clear();
    } else {
      printf("Tree %s does not exist\n",treeName);
    }
    gObjectTable->Print();
  }
