/// ~~~{.C}

#include <iostream>
#include <stdlib.h> 
#include <sys/stat.h>
#include <time.h>

#include <TROOT.h>
#include <TSystem.h>
#include <TDirectory.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TList.h>
#include <TString.h>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>

void CEPBuffersToList(TString input_dirname, Int_t filter=-1)
{
    // file extensions
    TString output_prefix("emcal_");
    const char *file_ext = ".root";
    
    // tree and branch name
    TString tree_name = "CEP";
    TString cep_evts_branchname = "CEPEvents";
    TString cep_raw_evts_branchname = "CEPRawEvents";

    TSystemDirectory input_dir(input_dirname, input_dirname);
    TList *input_files = input_dir.GetListOfFiles();


    TChain *cep_tree = new TChain(tree_name);
    if(input_files && (!input_dirname.EndsWith(".root")) ) {
        std::cout << "\nReading \"*" << file_ext << "\" files from \"" << input_dirname << "\"..." << std::endl;
        TSystemFile *file;
        TString fname;
        TIter next(input_files);
        while((file = (TSystemFile*)next())) {
            fname = file->GetName();
            if(!file->IsDirectory() && fname.EndsWith(file_ext)) {
                cep_tree->Add(input_dirname + fname);
            }
        }
    } else if(input_dirname.EndsWith(".root")) cep_tree->Add(input_dirname);
    else { printf("<E> Input file must end with .txt"); gSystem->Exit(0); } 

    CEPRawEventBuffer* cep_raw_evt = 0x0;
    CEPEventBuffer* cep_evt = 0x0;

    cep_tree->SetBranchAddress(cep_evts_branchname,     &cep_evt);
    cep_tree->SetBranchAddress(cep_raw_evts_branchname, &cep_raw_evt);
    // event number is a global number which is the recall variable for
    // every detector, track etc
    UInt_t event_nb;


    TFile* list_file = new TFile((output_prefix+"feature_plots.root").Data(), "RECREATE");
    TList* hist_list = new TList();

    // event level features:
    TH1F* evt_n_calo_tracks = new TH1F("n_calo_tracks", "", 41, 0, 40);
    TH1F* evt_tot_emc_ampl = new TH1F("tot_emc_ampl", "", 100, 0, 10.);
    TH1F* evt_tot_emc_time = new TH1F("tot_emc_time", "", 100, 0, 2.5e-5);
    TH1F* evt_tot_phos_ampl = new TH1F("tot_phos_ampl", "", 100, 0, 0.4);
    TH1F* evt_tot_phos_time = new TH1F("tot_phos_time", "", 100, 0, 0.0001);
    
    // emc
    TH1F* emcal_amplidude = new TH1F("emcal_amplidude", "", 100, 0, 1.);
    TH1F* emcal_time = new TH1F("emcal_time", "", 100, -2e-6, 2e-6);
    
    // phos
    TH1F* phos_amplidude = new TH1F("phos_amplidude", "", 100, 0, 0.4);
    TH1F* phos_time = new TH1F("phos_time", "", 100, 0, 1e-6);

    // calo-cluster
    TH1F* cc_energy = new TH1F("calo_cluster_energy", "", 100, 0, 1);
    TH1F* cc_shapeDispersion = new TH1F("calo_cluster_shape_dispersion", "", 100, 0, 3);
    TH1F* cc_chi2 = new TH1F("calo_cluster_chi2", "", 100, -2, 2);
    TH1F* cc_cpvdist = new TH1F("calo_cluster_cpvdist", "", 100, 0, 1500);

    Int_t cu, mode;
    Bool_t isDG, isNDG;
    Int_t nseltracks;

    // for display purposes only!
    Float_t progress = 0.0;
    Int_t barWidth = 70;
    /////////////////////////////
    std::cout << "\nReading " << cep_tree->GetEntries() << " events: " << std::endl;

    Int_t lhc16_filter = -999;
    CEPRawADBuffer* ad      = 0x0;
    CEPRawFMDBuffer* fmd    = 0x0;
    CEPRawV0Buffer* v0      = 0x0;
    CEPRawCaloBuffer* emcal = 0x0;
    CEPRawCaloBuffer* phos  = 0x0;
    Int_t n_tracks = 2;
    if (filter==99) n_tracks=3;
    for (UInt_t ii(0); ii<cep_tree->GetEntries(); ii++)
    {
        // display purposes only ////////////////////
        progress = float(ii)/float(cep_tree->GetEntries());
        std::cout << "[";
        Int_t pos = barWidth * progress;
        for (Int_t i(0); i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();
        // display purposes only ////////////////////

        cep_tree->GetEntry(ii);
        if (!cep_evt) {
            std::cout << "Event number " << ii << " cannot be found!" << std::endl;
            gSystem->Exit(1);
        }
        event_nb = ii;

        /* cu: 
         *      ein bitword mit dem man den Filter steuern kann, für Simulationen ist ein
         *      Wert von 74 geeignet.
         *
         * isDG: 
         *      wird dir am Ende sagen, ob das event ev ein CEP Kandidat ist
         *
         * nseltracks:
         *      ist dann die Anzahl tracks im event 
         */ 
        if (filter==0)      { cu=74; mode=0; }     // !V0
        else if (filter==1) { cu=65539; mode=0; }  // checkSPD hard
        else if (filter==2) { cu=65539; mode=1; }  // maxdnclu=maxdnfchips=maxdnfFOchips=1e5
        else if (filter==3) { cu=65539; mode=2; }  // maxnSingle=1
        else if (filter==4) { cu=65539; mode=3; }  // maxnSingle=2
        else                { cu=99; mode=0;}      // !AD (-1: keep all, >5 keep only passing)
        nseltracks = LHC16Filter(cep_evt,kFALSE,cu,isDG,isNDG,mode); 
        if ((filter>=0) && (filter!=99) && (isDG==kFALSE || nseltracks!=n_tracks)) continue;
        if ((filter==99) && (isDG==kFALSE || nseltracks<n_tracks)) continue;

        // initialize charge_sum with 0 for every new event
        Int_t evt_charge_sum_var = 0;
        
        evt_n_calo_tracks->Fill(cep_raw_evt->GetnCaloClusterTotal());

        evt_tot_emc_ampl->Fill(cep_raw_evt->GetTotalEMCAmplitude());
        evt_tot_emc_time->Fill(cep_raw_evt->GetTotalEMCTime());
        evt_tot_phos_ampl->Fill(cep_raw_evt->GetTotalPHOSAmplitude());
        evt_tot_phos_time->Fill(cep_raw_evt->GetTotalPHOSTime());

        // EMCAL
        emcal = cep_raw_evt->GetRawEMCalBuffer();
        for (UInt_t kk(0); kk<emcal->GetNCells(); kk++)
        {
            emcal_amplidude->Fill(emcal->GetCaloCellAmplitude(kk));
            emcal_time->Fill(emcal->GetCaloCellTime(kk));
        }
        // PHOS
        phos = cep_raw_evt->GetRawPHOSBuffer();
        for (UInt_t kk(0); kk<phos->GetNCells(); kk++)
        {
            phos_amplidude->Fill(phos->GetCaloCellAmplitude(kk));
            phos_time->Fill(phos->GetCaloCellTime(kk));
        }
        // Calo Clusters 
        CEPRawCaloClusterTrack* rawCaloCluster = 0x0;
        for (UInt_t kk(0); kk<cep_raw_evt->GetnCaloClusterTotal(); kk++)
        {
            rawCaloCluster = cep_raw_evt->GetCaloClusterTrack(kk);
            if (!rawCaloCluster) break;
            // put here all track info
            cc_energy->Fill(rawCaloCluster->GetCaloClusterE());
            cc_shapeDispersion->Fill(rawCaloCluster->GetCaloClusterShapeDispersion());
            cc_chi2->Fill(rawCaloCluster->GetCaloClusterChi2());
            cc_cpvdist->Fill(rawCaloCluster->GetCaloClusterCPVDist());
        }
    }
    // cursor of status display has to move to the next line
    std::cout << std::endl;
    std::cout << "100%:   conversion finished " << std::endl;
    ///////////////////////

    hist_list->Add(evt_n_calo_tracks);
    hist_list->Add(evt_tot_emc_ampl);
    hist_list->Add(evt_tot_emc_time);
    hist_list->Add(evt_tot_phos_ampl);
    hist_list->Add(evt_tot_phos_time);

    hist_list->Add(emcal_amplidude);
    hist_list->Add(emcal_time);

    hist_list->Add(phos_amplidude);
    hist_list->Add(phos_time);

    hist_list->Add(cc_energy);
    hist_list->Add(cc_shapeDispersion);
    hist_list->Add(cc_chi2);
    hist_list->Add(cc_cpvdist);
 
    list_file->cd();
    hist_list->Write();
    list_file->Close();
    delete list_file;

    std::cout << "\nSaving the TList in " << (output_prefix+"feature_plots.root").Data() << std::endl;
}

Int_t is_full_recon(CEPEventBuffer* cepevt)
{
    if (!cepevt) {
        std::cout << "Event does not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }
    // extract info if event is fully reconstruced
    // by the tracks in the TPC or not 
    TLorentzVector lv_MCparticle;       // initialized by (0., 0., 0., 0.)
    TLorentzVector lv_trks_total;       // initialized by (0., 0., 0., 0.)
    TLorentzVector lv_trk_temp;         // initialized by (0., 0., 0., 0.)
    lv_MCparticle = cepevt->GetMCParticle();
    // looping through the detected tracks
    // to see if they match the 4-mom of the
    // original particle -> if so the whole evt is
    // detected
    CEPTrackBuffer* trk = 0x0;
    for (int kk(0); kk<cepevt->GetnTracks(); kk++) {
        trk = cepevt->GetTrack(kk); 
        if (!trk) break;

        lv_trk_temp.SetVectM(trk->GetMCMomentum(), trk->GetMCMass());
        lv_trks_total += lv_trk_temp;
    }
    if ( abs_val(lv_MCparticle.M() - lv_trks_total.M()) < 10e-3 ) {
        /* std::cout << "CEP Particle: " << std::endl; */
        /* print_lv(lv_MCparticle); */
        /* std::cout << cepevt->GetnTracks() << " Particles: "; */
        /* if (cepevt->GetnTracks() > 2 ) std::cout << "           <------------- MORE THAN 2 TRACKS! "; */
        /* std::cout << "\n"; */
        /* print_lv(lv_trks_total); */
        /* std::cout << "\n--------------------------------------------------------" << std::endl; */
        return 1;
    } else return 0;
}

double abs_val(double val)
{
    if (val > 0.) return val;
    else return -val;
}

void print_lv(TLorentzVector lv)
{
    std::cout << "px: " << lv.Px() << std::endl;
    std::cout << "py: " << lv.Py() << std::endl;
    std::cout << "pz: " << lv.Pz() << std::endl;
    std::cout << "m:  " << lv.M() << std::endl;
}


