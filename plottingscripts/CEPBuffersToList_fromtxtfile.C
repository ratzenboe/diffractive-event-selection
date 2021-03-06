/// ~~~{.C}

#include <iostream>
#include <stdlib.h> 
#include <sys/stat.h>
#include <time.h>
#include <algorithm>    // std::random_shuffle

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

void CEPBuffersToList(TString input_dirname, TString path_to_evt_id_txt, TString outname,
        Bool_t use_bayes_proba=false)
{
    // file extensions
    std::vector<Int_t> evt_id_vec;
    evt_id_vec.clear();
    if (path_to_evt_id_txt.EndsWith(".txt")) {
        std::ifstream txtfile(path_to_evt_id_txt.Data());
        Int_t inp_int;
    	while (txtfile >> inp_int) evt_id_vec.push_back(inp_int);
    } else { printf("<E> Input file must end with .txt"); gSystem->Exit(0); }
    if (evt_id_vec.size() == 0) { printf("<W> No event fount in %s\n", path_to_evt_id_txt); 
                                  gSystem->Exit(0); } 
    else printf("%i events in %s", evt_id_vec.size(), path_to_evt_id_txt.Data());

    const char *file_ext = ".root";
    // tree and branch name
    TString tree_name = "CEP";
    TString cep_evts_branchname = "CEPEvents";
    TString cep_raw_evts_branchname = "CEPRawEvents";

    TSystemDirectory input_dir(input_dirname, input_dirname);
    TList *input_files = input_dir.GetListOfFiles();

    TChain *cep_tree = new TChain(tree_name);
    if(input_files) {
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
    }

    CEPRawEventBuffer* cep_raw_evt = 0x0;
    CEPEventBuffer* cep_evt = 0x0;

    cep_tree->SetBranchAddress(cep_evts_branchname,     &cep_evt);
    cep_tree->SetBranchAddress(cep_raw_evts_branchname, &cep_raw_evt);
    // event number is a global number which is the recall variable for
    // every detector, track etc
    if (!outname.EndsWith(".root")) outname += ".root";
    TFile* list_file = new TFile((outname).Data(), "RECREATE");
    TList* hist_list = new TList();

    // event level features:
    TH1F* evt_n_tracks = new TH1F("n_tracks", "", 10, 0, 10);
    TH1F* evt_n_tracklets = new TH1F("n_tracklets", "", 101, 0, 100);
    TH1F* evt_n_singles = new TH1F("n_singles", "", 21, 0, 20);
    TH1F* evt_n_tracks_total = new TH1F("n_tracks_total", "", 100, 0, 400);
    TH1F* evt_n_residuals = new TH1F("n_residuals", "", 100, 0, 100);
    TH1F* evt_n_calo_tracks = new TH1F("n_calo_tracks", "", 100, 0, 100);
    TH1F* evt_tot_ad_mult = new TH1F("tot_ad_mult", "", 100, 0, 10.);
    TH1F* evt_tot_ad_time = new TH1F("tot_ad_time", "", 1000, -2000, 2500);
    TH1F* evt_tot_ad_charge = new TH1F("tot_ad_charge", "", 100, 0, 6.);
    TH1F* evt_tot_fmd_mult = new TH1F("tot_fmd_mult", "", 100, 540, 570);
    TH1F* evt_tot_v0_mult = new TH1F("tot_v0_mult", "", 100, 0, 0.5);
    TH1F* evt_tot_v0_charge = new TH1F("tot_v0_charge", "", 100, 0, 5.);
    TH1F* evt_tot_v0_time = new TH1F("tot_v0_time", "", 100, 0, 5.);
    TH1F* evt_tot_v0_sig_width = new TH1F("tot_v0_sig_width", "", 100, 0, 40);
    TH1F* evt_tot_emc_ampl = new TH1F("tot_emc_ampl", "", 100, 0, 1.);
    TH1F* evt_tot_emc_time = new TH1F("tot_emc_time", "", 100, 0, 2.5-5);
    TH1F* evt_tot_phos_ampl = new TH1F("tot_phos_ampl", "", 100, 0, 0.4);
    TH1F* evt_tot_phos_time = new TH1F("tot_phos_time", "", 100, 0, 0.0001);
    TH1F* evt_n_v0s = new TH1F("n_v0s", "", 18, 0, 17);
    TH1F* evt_charge_sum = new TH1F("charge_sum", "", 40, -10, 10);

    // hlt = high level track
    TH1F* trk_tof_bunch_crossing = new TH1F("tof_bunch_crossing", "", 100, 0, 100);
    TH1F* trk_dca_vtx_z = new TH1F("dca_vtx_z", "", 100, -30, 30);
    TH1F* trk_pt = new TH1F("pt", "", 100, 0, 10);
    TH1F* trk_eta = new TH1F("eta", "", 100, -1.7, 1.7);
    TH1F* trk_phi = new TH1F("phi", "", 100, -3.5, 3.5);
    TH1F* trk_P = new TH1F("P", "", 100, 0, 10);
    TH1F* trk_pid_global = new TH1F("pid_global", "", 100, 0, 100);
    TH1F* trk_tpc_status = new TH1F("pid_tpc_status", "", 2, 0, 1);
    TH1F* trk_tpc_signal = new TH1F("pid_tpc_signal", "", 100, 0, 100.);
    TH1F* trk_tpc_n_sigma_pion = new TH1F("pid_tpc_n_sigma_pion", "", 100, -5, 15.);
    TH1F* trk_tpc_n_sigma_kaon = new TH1F("pid_tpc_n_sigma_kaon", "", 100, -17, 20);
    TH1F* trk_tpc_n_sigma_proton = new TH1F("pid_tpc_n_sigma_proton", "", 100, -18, 4.);
    TH1F* trk_tpc_proba_pion = new TH1F("pid_tpc_proba_pion", "", 100, 0, 1);
    TH1F* trk_tpc_proba_kaon = new TH1F("pid_tpc_proba_kaon", "", 100, 0, 1);
    TH1F* trk_tpc_proba_proton = new TH1F("pid_tpc_proba_proton", "", 100, 0, 1);
    TH1F* trk_tof_signal = new TH1F("pid_tof_signal", "", 100, 0, 2.5e4);
    TH1F* trk_tof_status = new TH1F("pid_tof_status", "", 2, 0, 1);
    TH1F* trk_tof_n_sigma_pion = new TH1F("pid_tof_n_sigma_pion", "", 100, -4, 4);
    TH1F* trk_tof_n_sigma_kaon = new TH1F("pid_tof_n_sigma_kaon", "", 100, -35, 5.);
    TH1F* trk_tof_n_sigma_proton = new TH1F("pid_tof_n_sigma_proton", "", 100, -40, 4.); 
    TH1F* trk_tof_proba_pion = new TH1F("pid_tof_proba_pion", "", 100, 0, 1);
    TH1F* trk_tof_proba_kaon = new TH1F("pid_tof_proba_kaon", "", 100, 0, 1);
    TH1F* trk_tof_proba_proton = new TH1F("pid_tof_proba_proton", "", 100, 0, 1);
    TH1F* trk_pid_bayes_status = new TH1F("pid_bayes_status", "", 20, 0, 11);
    TH1F* trk_pid_bayes_proba_e = new TH1F("pid_bayes_proba_e", "", 100, 0, 1);
    TH1F* trk_pid_bayes_proba_muon = new TH1F("pid_bayes_proba_muon", "", 100, 0, 1);
    TH1F* trk_pid_bayes_proba_pion = new TH1F("pid_bayes_proba_pion", "", 100, 0, 1);
    TH1F* trk_pid_bayes_proba_kaon = new TH1F("pid_bayes_proba_kaon", "", 100, 0, 1);
    TH1F* trk_pid_bayes_proba_proton = new TH1F("pid_bayes_proba_proton", "", 100, 0, 1);
    TH1F* trk_length = new TH1F("track_length", "", 100, 0, 600.);
    TH1F* trk_global_chi2 = new TH1F("global_chi2", "", 100, 0, 100);
    TH1F* trk_golden_chi2 = new TH1F("golden_chi2", "", 100, -2, 15.);
    TH1F* trk_its_chi2 = new TH1F("its_chi2", "", 100, 0, 100);
    TH1F* trk_tpc_chi2 = new TH1F("tpc_chi2", "", 100, 0, 500);
    TH1F* trk_hmpid_signal = new TH1F("hmpid_signal", "", 100, -20, 10);
    TH1F* trk_its_signal = new TH1F("its_signal", "", 100, 0, 300);
    TH1F* trk_trd_signal = new TH1F("trd_signal", "", 100, 0, 3.);
    TH1F* trk_xy_impact = new TH1F("xy_impact", "", 100, -3, 3);
    TH1F* trk_z_impact = new TH1F("z_impact", "", 100, -4, 4);
    TH1F* trk_its_ncls = new TH1F("its_n_cls", "", 100, 0, 64);
    TH1F* trk_tpc_ncls = new TH1F("tpc_n_cls", "", 100, 0, 200);
    TH1F* trk_trd_ncls = new TH1F("trd_n_cls", "", 100, 0, 160);
    TH1F* trk_tpc_n_shared_cls = new TH1F("tpc_n_shared_cls", "", 100, 0, 160);

    // ad
    TH1F* ad_mult = new TH1F("ad_multiplicity", "", 100, 0, 8.);
    TH1F* ad_time = new TH1F("ad_time", "", 100, 40., 200.);
    TH1F* ad_charge = new TH1F("ad_charge", "", 100, 0, 4.);

    // fmd
    TH1F* fmd_mult = new TH1F("fmd_multiplicity", "", 100, 0, 15.);

    // v0
    TH1F* v0_mult = new TH1F("v0_multiplicity", "", 100, 0, 1.);
    TH1F* v0_charge = new TH1F("v0_charge", "", 100, 0, 4.);
    TH1F* v0_time = new TH1F("v0_time", "", 100, 20., 40);
    TH1F* v0_sig_width = new TH1F("v0_sig_width", "", 100, 0, 31.);

    // emc
    TH1F* emcal_amplidude = new TH1F("emcal_amplidude", "", 100, 0., 1.);
    TH1F* emcal_time = new TH1F("emcal_time", "", 100, 0, 2e-6);
    
    // phos
    TH1F* phos_amplidude = new TH1F("phos_amplidude", "", 100, 0, 0.4);
    TH1F* phos_time = new TH1F("phos_time", "", 100, 0, 1e-6);

    // calo-cluster
    TH1F* cc_energy = new TH1F("calo_cluster_energy", "", 100, 0, 1.);
    TH1F* cc_shapeDispersion = new TH1F("calo_cluster_shape_dispersion", "", 100, 0, 3.);
    TH1F* cc_chi2 = new TH1F("calo_cluster_chi2", "", 10, -2, 2);
    TH1F* cc_cpvdist = new TH1F("calo_cluster_cpvdist", "", 1000, 0, 1500);

    // for display purposes only!
    Float_t progress = 0.0;
    Int_t barWidth = 70;
    /////////////////////////////
    Int_t lhc16_filter = -999;
    CEPRawADBuffer* ad      = 0x0;
    CEPRawFMDBuffer* fmd    = 0x0;
    CEPRawV0Buffer* v0      = 0x0;
    CEPRawCaloBuffer* emcal = 0x0;
    CEPRawCaloBuffer* phos  = 0x0;
    for (Int_t ii(0); ii< evt_id_vec.size(); ii++)
    {
        // display purposes only ////////////////////
        progress = float(ii)/float(evt_id_vec.size());
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


        cep_tree->GetEntry(evt_id_vec[ii]);
        if (!cep_evt) {
            std::cout << "Event number " << evt_id_vec[ii] << " cannot be found!" << std::endl;
            gSystem->Exit(1);
        }
        std::vector<Int_t> part_vec;
        part_vec.resize(cep_evt->GetnTracks());
        if (cep_evt->GetnTracks()>2) {
            Bool_t kPiPlus(false), kPiMinus(false);
            for (UInt_t kk(0); kk<cep_evt->GetnTracks(); kk++){
                if (cep_evt->GetTrack(kk)->GetMCPID() == 211) kPiPlus=true;
                if (cep_evt->GetTrack(kk)->GetMCPID() == -211) kPiMinus=true;
            }
            if (!kPiPlus || !kPiMinus) continue;
            // create a vector with lenght cep_evt->GetnTracks()
            for (UInt_t kk(0); kk<part_vec.size(); kk++) part_vec[kk] = kk;
            while(true) {
                std::random_shuffle( part_vec.begin(), part_vec.end() );
                Int_t charge_sum = cep_evt->GetTrack(part_vec[0])->GetChargeSign() + 
                    cep_evt->GetTrack(part_vec[1])->GetChargeSign();
                Int_t pid0pi, pid1pi;
                pid0pi = !use_bayes_proba ? (abs(cep_evt->GetTrack(part_vec[0])->GetMCPID())==211) : 
                    (cep_evt->GetTrack(part_vec[0])->GetPIDBayesProbability(AliPID::kPion) > 0.9);
                pid1pi = !use_bayes_proba ? (abs(cep_evt->GetTrack(part_vec[1])->GetMCPID())==211) : 
                    (cep_evt->GetTrack(part_vec[1])->GetPIDBayesProbability(AliPID::kPion) > 0.9);
                if ( charge_sum==0 && pid0pi && pid1pi ) break;
            }
        }
  
        // initialize charge_sum with 0 for every new event
        Int_t evt_charge_sum_var = 0;
        
        // HL track info
        CEPTrackBuffer* trk = 0x0;
        CEPRawTrackBuffer* rawTrack = 0x0;
        TVector3 v;
        UInt_t hlt_kk(0);
        Int_t track_nb(0);
        for (UInt_t kk(0); kk<2; kk++)
        {
            if (cep_evt->GetnTracks()>2) track_nb=part_vec[kk];
            else track_nb = kk;
            trk = cep_evt->GetTrack(track_nb);
            if (!trk) break;
            // put here all track info
            trk_tof_bunch_crossing->Fill(trk->GetTOFBunchCrossing());
            trk_dca_vtx_z->Fill(trk->GetZv());
            // momentum 
            v = trk->GetMomentum(); 
            trk_pt->Fill(v.Pt());
            trk_eta->Fill(v.Eta());
            trk_phi->Fill(v.Phi());
            trk_P->Fill(v.Mag());
            // PID
            trk_pid_global->Fill(trk->GetPID());
            // TPC
            trk_tpc_status->Fill(trk->GetPIDTPCStatus());
            trk_tpc_signal->Fill(trk->GetPIDTPCSignal());
            // n sigma
            trk_tpc_n_sigma_pion->Fill(trk->GetPIDTPCnSigma(AliPID::kPion));
            trk_tpc_n_sigma_kaon->Fill(trk->GetPIDTPCnSigma(AliPID::kKaon));
            trk_tpc_n_sigma_proton->Fill(trk->GetPIDTPCnSigma(AliPID::kProton));
            // probability
            trk_tpc_proba_pion->Fill(trk->GetPIDTPCProbability(AliPID::kPion));
            trk_tpc_proba_kaon->Fill(trk->GetPIDTPCProbability(AliPID::kKaon));
            trk_tpc_proba_proton->Fill(trk->GetPIDTPCProbability(AliPID::kProton));
            // TOF 
            trk_tof_status->Fill(trk->GetPIDTOFStatus());
            trk_tof_signal->Fill(trk->GetPIDTOFSignal());
            // n sigma
            trk_tof_n_sigma_pion->Fill(trk->GetPIDTOFnSigma(AliPID::kPion));
            trk_tof_n_sigma_kaon->Fill(trk->GetPIDTOFnSigma(AliPID::kKaon));
            trk_tof_n_sigma_proton->Fill(trk->GetPIDTOFnSigma(AliPID::kProton));
            // probability
            trk_tof_proba_pion->Fill(trk->GetPIDTOFProbability(AliPID::kPion));
            trk_tof_proba_kaon->Fill(trk->GetPIDTOFProbability(AliPID::kKaon));
            trk_tof_proba_proton->Fill(trk->GetPIDTOFProbability(AliPID::kProton));
            // Bayes
            trk_pid_bayes_status->Fill(trk->GetPIDBayesStatus());
            trk_pid_bayes_proba_pion->Fill(trk->GetPIDBayesProbability(AliPID::kPion));
            trk_pid_bayes_proba_kaon->Fill(trk->GetPIDBayesProbability(AliPID::kKaon));
            trk_pid_bayes_proba_proton->(trk->GetPIDBayesProbability(AliPID::kProton));

            trk_pid_bayes_proba_e->Fill(trk->GetPIDBayesProbability(AliPID::kElectron));
            trk_pid_bayes_proba_muon->Fill(trk->GetPIDBayesProbability(AliPID::kMuon));

            trk_golden_chi2->Fill(trk->GetGoldenChi2());
            // charge sign
            evt_charge_sum_var += trk->GetChargeSign();
            // clusters
            trk_its_ncls->Fill(trk->GetITSncls());
            trk_tpc_ncls->Fill(trk->GetTPCncls());
            trk_trd_ncls->Fill(trk->GetTRDncls());
            trk_tpc_n_shared_cls->Fill(trk->GetTPCnclsS());

            // raw track features
            hlt_kk = trk->GetTrackindex();
            rawTrack = cep_raw_evt->GetTrack(hlt_kk);
            if (!rawTrack) break;
            // put here all track info
            trk_length->Fill(rawTrack->GetTrackLength());
            trk_global_chi2->Fill(rawTrack->GetGlobalChi2());
            trk_its_chi2->Fill(rawTrack->GetITSChi2());
            trk_tpc_chi2->Fill(rawTrack->GetTPCChi2());

            trk_its_signal->Fill(rawTrack->GetPIDITSsig());
            trk_hmpid_signal->Fill(rawTrack->GetPIDHMPsig());
            trk_trd_signal->Fill(rawTrack->GetPIDTRDsig());

            trk_xy_impact->Fill(rawTrack->GetImpactXY());
            trk_z_impact->Fill(rawTrack->GetImpactZ());
        }
        evt_charge_sum->Fill(evt_charge_sum_var);
        // Event  
        // event info contains charge sum, hence we have to first loop
        // over the tracks to gain this information
        evt_n_tracks->Fill(cep_evt->GetnTracks());
        evt_n_tracklets->Fill(cep_evt->GetnTracklets());
        evt_n_singles->Fill(cep_evt->GetnSingles()); 
        evt_n_residuals->Fill(cep_evt->GetnResiduals());
        evt_n_tracks_total->Fill(cep_evt->GetnTracksTotal()); 

        evt_n_calo_tracks->Fill(cep_raw_evt->GetnCaloClusterTotal());

        evt_tot_ad_mult->Fill(cep_raw_evt->GetTotalADMult());
        evt_tot_ad_time->Fill(cep_raw_evt->GetTotalADTime());
        evt_tot_ad_charge->Fill(cep_raw_evt->GetTotalADCharge());

        evt_tot_fmd_mult->Fill(cep_raw_evt->GetTotalFMDMult());

        evt_tot_v0_mult->Fill(cep_raw_evt->GetTotalV0Mult());
        evt_tot_v0_time->Fill(cep_raw_evt->GetTotalV0Time());
        evt_tot_v0_charge->Fill(cep_raw_evt->GetTotalV0Charge());
        evt_tot_v0_sig_width->Fill(cep_raw_evt->GetTotalV0SigWidth());
       
        evt_tot_emc_ampl->Fill(cep_raw_evt->GetTotalEMCAmplitude());
        evt_tot_emc_time->Fill(cep_raw_evt->GetTotalEMCTime());
        evt_tot_phos_ampl->Fill(cep_raw_evt->GetTotalPHOSAmplitude());
        evt_tot_phos_time->Fill(cep_raw_evt->GetTotalPHOSTime());

        evt_n_v0s->Fill(cep_evt->GetnV0());

        // AD
        ad = cep_raw_evt->GetRawADBuffer();
        Double_t time_var = 0;
        for (UInt_t kk(0); kk<ad->GetNCells(); kk++)
        {
            ad_mult->Fill(ad->GetADMultiplicity(kk));
	    time_var = ad->GetADTime(kk);
	    if (time_var!=-1024.) ad_time->Fill(time_var);
            ad_charge->Fill(ad->GetADCharge(kk));
        }
        // FMD
        fmd = cep_raw_evt->GetRawFMDBuffer();
        for (UInt_t kk(0); kk<fmd->GetFMDnCells(); kk++)
        {
            fmd_mult->Fill(fmd->GetFMDCellMultiplicity(kk));
        }
        // V0 
        v0 = cep_raw_evt->GetRawV0Buffer();
        for (UInt_t kk(0); kk<v0->GetNCells(); kk++)
        {
            v0_mult->Fill(v0->GetV0Multiplicity(kk));
       	    time_var = v0->GetV0Time(kk);
	    if (time_var!=-1024.) v0_time->Fill(time_var);
            v0_charge->Fill(v0->GetV0Charge(kk));
            v0_sig_width->Fill(v0->GetV0Width(kk));
        }
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

    hist_list->Add(evt_n_tracks);
    hist_list->Add(evt_n_singles);
    hist_list->Add(evt_n_tracks_total);
    hist_list->Add(evt_n_residuals);
    hist_list->Add(evt_n_tracklets);
    hist_list->Add(evt_n_calo_tracks);
    hist_list->Add(evt_tot_ad_mult);
    hist_list->Add(evt_tot_ad_time);
    hist_list->Add(evt_tot_fmd_mult);
    hist_list->Add(evt_tot_v0_mult);
    hist_list->Add(evt_tot_v0_time);
    hist_list->Add(evt_tot_v0_charge);
    hist_list->Add(evt_tot_v0_sig_width);
    hist_list->Add(evt_tot_emc_ampl);
    hist_list->Add(evt_tot_emc_time);
    hist_list->Add(evt_tot_phos_ampl);
    hist_list->Add(evt_tot_phos_time);
    hist_list->Add(evt_n_v0s);
    hist_list->Add(evt_charge_sum);

    // hlt = high level track
    hist_list->Add(trk_tof_bunch_crossing);
    hist_list->Add(trk_dca_vtx_z);
    hist_list->Add(trk_pt);
    hist_list->Add(trk_eta);
    hist_list->Add(trk_phi);
    hist_list->Add(trk_P);
    hist_list->Add(trk_pid_global);
    hist_list->Add(trk_tpc_status);
    hist_list->Add(trk_tpc_signal);
    hist_list->Add(trk_tpc_n_sigma_pion);
    hist_list->Add(trk_tpc_n_sigma_kaon);
    hist_list->Add(trk_tpc_n_sigma_proton);
    hist_list->Add(trk_tpc_proba_pion);
    hist_list->Add(trk_tpc_proba_kaon);
    hist_list->Add(trk_tpc_proba_proton);
    hist_list->Add(trk_tof_status);
    hist_list->Add(trk_tof_signal);
    hist_list->Add(trk_tof_n_sigma_pion);
    hist_list->Add(trk_tof_n_sigma_kaon);
    hist_list->Add(trk_tof_n_sigma_proton);
    hist_list->Add(trk_tof_proba_pion);
    hist_list->Add(trk_tof_proba_kaon);
    hist_list->Add(trk_tof_proba_proton);
    hist_list->Add(trk_pid_bayes_status);
    hist_list->Add(trk_pid_bayes_proba_e);
    hist_list->Add(trk_pid_bayes_proba_muon);
    hist_list->Add(trk_pid_bayes_proba_pion);
    hist_list->Add(trk_pid_bayes_proba_kaon);
    hist_list->Add(trk_pid_bayes_proba_proton);
    hist_list->Add(trk_length);
    hist_list->Add(trk_global_chi2);
    hist_list->Add(trk_golden_chi2);
    hist_list->Add(trk_its_chi2);
    hist_list->Add(trk_tpc_chi2);
    hist_list->Add(trk_hmpid_signal);
    hist_list->Add(trk_its_signal);
    hist_list->Add(trk_trd_signal);
    hist_list->Add(trk_xy_impact);
    hist_list->Add(trk_z_impact);
    hist_list->Add(trk_its_ncls);
    hist_list->Add(trk_tpc_ncls);
    hist_list->Add(trk_trd_ncls);
    hist_list->Add(trk_tpc_n_shared_cls);

    hist_list->Add(ad_mult);
    hist_list->Add(ad_time);
    hist_list->Add(ad_charge);

    hist_list->Add(fmd_mult);

    hist_list->Add(v0_mult);
    hist_list->Add(v0_time);
    hist_list->Add(v0_charge);
    hist_list->Add(v0_sig_width);

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

    std::cout << "\nSaving the TList in " << (outname).Data() << std::endl;
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


