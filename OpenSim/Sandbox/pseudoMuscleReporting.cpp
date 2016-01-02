
#include <OpenSim/OpenSim.h>

main() {
    // A study is the top level component that contains the model and other 
    // computational components. It maintains "results" as data tables.
    // It is a replacement for Tool and the system and storgae owning
    // elements of the model
    Study muscleStudy;

    Model model("subject_02.osim");
    // the exact states trajectory generated by an OpenSim simulation
    StatesTrajectory states("states.ost");
    OutputReporter muscleReporter;
    muscleReporter.addOutput("/muscles/vasti_r/fiber_length");
    muscleReporter.addOutputs("/muscles/*/passive_fiber_force");
    muscleReporter.addOutputs("muscles/*/moment_arm");

    // Add the model to the study
    muscleStudy.addComponent(model);
    // Add reporters to the Study a NOT a Model
    muscleStudy.addReporter(muscleReporter);

    // initialize the Study and underlying system
    muscleStudy.initSystem();

    // realize the study to the reporting stage (time->pos->vel->acc->report)
    // so that all associated reporters can be evaluated
    for (const auto& state : states)
        muscleStudy.realizeReport(state);

    // in memory results to be used for further processing
    TimeSeriesTable results = muscleReporter.getReport();
    // or write it to file
    FileAdapter fileAdapter;
    fileAdapter.write("muscleResults.csv", results);
}
