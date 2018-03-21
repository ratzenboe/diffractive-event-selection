/// ~~~{.C}

#include <iostream>
#include <stdlib.h> 
#include <sys/stat.h>
#include <time.h>

#include <TROOT.h>
#include <TSystem.h>
#include <TObjArray.h>
#include <TDirectory.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TList.h>
#include <TString.h>
#include <TFile.h>

void MergeTLists(TString fname_1, TString fname_2, TString output_file)
{
    if (!output_file.EndsWith(".root")) { std::cout << "<E> Output path not valid\n"; gSystem->Exit(1);  }

    TFile* list_file = new TFile((output_file).Data(), "RECREATE");
    TList* hist_list = new TList();

    TFile* file_1 = TFile::Open(fname_1);
    TFile* file_2 = TFile::Open(fname_2);
    if (!file_1 || !file_2) { std::cout << "<E> File paths not found\n"; gSystem->Exit(1);  }

    // file extensions
    // 1: loop through one of the files to get all hists
    Double_t norm = 0.;
    TList* list_of_histnames = file_1->GetListOfKeys();
    if (!list_of_histnames) { std::cout << "<E> No keys found in file\n"; gSystem->Exit(1);  }
    TIter next(list_of_histnames);
    TKey* key;
    TObject* obj;
    while ( key = (TKey*)next() ) {
        obj = (TH1F*)key->ReadObj();
        if (!obj->InheritsFrom("TH1")) { std::cout << "The list contains NON-TH1 objects!\n"; }
        else std::cout << "Histo name: " << obj->GetName() << std::endl;
        // at first we clone the histogram to which we then add the remaining hists 
        // the Remove(0,5) removes the "full_"-chars in front of the histogram name
        TH1F* clone_hist_1 = (TH1F*)obj->Clone((TString(obj->GetName()).Remove(0,5)).Data());
        TH1F* clone_hist_2 = ((TH1F*)file_2->Get(obj->GetName()))->Clone((TString(obj->GetName()).Remove(0,5)+"_2").Data());
        Double_t integral_1 = clone_hist_1->Integral(); 
        Double_t integral_2 = clone_hist_2->Integral(); 
        clone_hist_1->Sumw2();
        if (integral_1>integral_2) { norm = integral_2 / integral_1; clone_hist_1->Scale(norm); }
        else { norm = integral_1 / integral_2; clone_hist_2->Scale(norm); }

        TRatioPlot* rp = new TRatioPlot(clone_hist_1, clone_hist_2);
        hist_list->Add(rp);
    }

    list_file->cd();
    hist_list->Write();
    list_file->Close();
    delete list_file;
    delete hist_list;

    std::cout << "\nSaving the merged TList in " << output_file.Data() << std::endl;
    std::cout << "\nEverything successful!" << std::endl;
}

