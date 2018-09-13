{
    // includes from root and alice-root classes
    gROOT->ProcessLine(Form(".include %s/include", gSystem->ExpandPathName("$ROOTSYS")));
    gROOT->ProcessLine(Form(".include %s/include", gSystem->ExpandPathName("$ALICE_ROOT")));
    gROOT->ProcessLine(Form(".include %s/include", gSystem->ExpandPathName("$ALICE_PHYSICS")));
    
    // includes from own class
    gInterpreter->AddIncludePath("/home/ratzenboe/Documents/diffractive-event-selection/recurrantNets/RNN/lib");
    // compile macro
    gROOT->ProcessLine(".L /home/ratzenboe/Documents/diffractive-event-selection/recurrantNets/RNN/lib/CEPfilters.C+g");
    // compile own classes 
    gROOT->ProcessLine(".L /home/ratzenboe/Documents/diffractive-event-selection/recurrantNets/RNN/lib/EventDef.cxx+g");
    gROOT->ProcessLine(".L /home/ratzenboe/Documents/diffractive-event-selection/recurrantNets/RNN/lib/EventStorage.cxx+g");
    gROOT->ProcessLine(".L /home/ratzenboe/Documents/diffractive-event-selection/recurrantNets/RNN/lib/CEPBGBase.cxx+g");
    gROOT->ProcessLine(".L /home/ratzenboe/Documents/diffractive-event-selection/recurrantNets/RNN/lib/PlotTask.cxx+g");
}
