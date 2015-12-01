# run_job

##INSTALLATION

1) In your terminal on Feynman, type:

    git clone https://github.com/tcoates3/run_job run_job

2) Navigate into the /run_job directory and create an directory called /Outputs:

    cd run_job
    mkdir Outputs

3) Then, in your ~/.bashrc file, add the following line:

    alias run_job="python /lustre/scratch/epp/ilc/tc297/run_job/run_job_files/run_job.py"

##USAGE

To use, make sure that the ILCsetup.sh file has been sourced, then type:

    run_job <sindarin file> <no. of events>

This (should) do everything right, since the methods for finding directories are now (relatively) robust. This command will create a folder within your /run_job/Outputs directory titled with the name of the sindarin file. Within this will be another folder named with timestamp of when the run_job command was issued, followed by the number of events it was run for.

IMPORTANT: Make sure that the number of events you specify in the command line is the same as the n_events line in the Sindarin file, otherwise errors may occur, including but not limited to: Mokka complaining that it was asked to to more events than it can fine, Whizard throwing errors (and consequently forcing Mokka to abort) and Gandalf inviting too many dwarves to your hobbit hole.
