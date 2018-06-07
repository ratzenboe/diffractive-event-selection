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

void MergeTLists(TString input_dirname)
{
    TFile* list_file = new TFile((input_dirname+"all_feature_plots.root").Data(), "RECREATE");
    TList* hist_list = new TList();

    // file extensions
    const char *file_ext = ".root";
    const char *file_begin = "feature_plots_";

    TObjArray* filearray = new TObjArray();
    filearray->SetOwner(kTRUE);
    TFile* rootfile = 0x0;
    
    // put the files that fit the begin&ext into the filearray 
    TSystemDirectory dir(input_dirname, input_dirname); 
    TList *files = dir.GetListOfFiles(); 
    if (files) { 
        TSystemFile *file; 
        TString fname; 
        TIter next(files); 
        while ((file=(TSystemFile*)next())) 
        { 
            fname = file->GetName(); 
            if (!file->IsDirectory() && fname.EndsWith(file_ext) 
                    && fname.BeginsWith(file_begin)) { 
                rootfile = TFile::Open((input_dirname+fname).Data());
                if (!rootfile){ printf("The file %s cannot be opened\n", (input_dirname+fname).Data()); }
                filearray->Add(rootfile);
            } 
        } 
    }

    std::cout << "\n" << filearray->GetEntries() << " files to merge:\n";
    for (UInt_t ii(0); ii<filearray->GetEntries(); ii++){
        std::cout << ((TFile*)filearray->At(ii))->GetName() << std::endl;
    }
    // 1: loop through one of the files to get all hists
    TFile* basefile = (TFile*)filearray->At(0);
    TList* list_of_histnames = basefile->GetListOfKeys();
    if (!list_of_histnames) { std::cout << "<E> No keys found in file\n"; gSystem->Exit(1);  }
    TIter next(list_of_histnames);
    TKey* key;
    TObject* obj;
    while ( key = (TKey*)next() ) {
        obj = (TH1F*)key->ReadObj();
        if (!obj->InheritsFrom("TH1")) { std::cout << "The list contains NON-TH1 objects!\n"; }
        else std::cout << "Histo name: " << obj->GetName() << std::endl;
        // at first we clone the histogram to which we then add the remaining hists 
        TH1F* clone_hist = (TH1F*)obj->Clone(("full_"+TString(obj->GetName())).Data());
        for (UInt_t ii(1); ii<filearray->GetEntries(); ii++){
            clone_hist->Add( (TH1F*)(((TFile*)filearray->At(ii))->Get(obj->GetName())) );
        }
        hist_list->Add(clone_hist);
    }



    list_file->cd();
    hist_list->Write();
    list_file->Close();
    delete list_file;
    delete hist_list;

    delete filearray;
    std::cout << "\nSaving the merged TList in " << (input_dirname+"all_feature_plots.root").Data() << std::endl;
    std::cout << "\nEverything successful!" << std::endl;

}

