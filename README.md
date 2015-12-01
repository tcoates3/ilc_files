# run_job

## INSTALLATION

1) In your terminal on Feynman, type:

    git clone https://github.com/tcoates3/run_job run_job

2) Then edit your .bashrc and add an alias:

    emacs ~/.bashrc &
    alias run_job="python <full path to run_job directory>/run_job/run_job_files/run_job.py"

3) In the terminal, to source the modified .bashrc, type:

    . ~/.bashrc

## USAGE

To use, make sure that the ILCsetup.sh file has been sourced, then type:

    run_job <sindarin file>

This (should) do everything right, since the methods for finding directories are now (relatively) robust. This command will create a folder within your /run_job/output directory titled with the name of the Sindarin file. Within this will be another folder named with timestamp of when the run_job command was issued, followed by the number of events it was run for.
