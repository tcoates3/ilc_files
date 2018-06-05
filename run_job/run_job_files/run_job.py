"""run_job

Usage:
  run_job <sin_file> -l <s, m or l>

Options:
"""

import argparse
import logging
import os
import pwd
import grp
import sys
import time
import subprocess

def submit_job(name, command, output_path, length='mps.short', queue='mps.q', binary='y'):
  job_id=subprocess.check_output(["qsub","-terse", "-jc", length,"-q",queue,"-b",binary,"-S","/bin/bash","-N",name,"-j","y","-o",output_path,"-cwd",command]).strip()
  logger.info('Submitted {} job: {}'.format(name, job_id))
  return job_id.strip()

def submit_dependent_job(name, command, output_path, parent_job_id, length='mps.short', queue='mps.q', binary='y'):
  job_id=subprocess.check_output(["qsub","-terse","-hold_jid",parent_job_id,"-jc",length,"-q",queue,"-b",binary,"-S","/bin/bash","-N",name,"-j","y","-o",output_path,"-cwd",command]).strip()
  logger.info('Submitted {} job: {}'.format(name, job_id))
  return job_id

if __name__ == '__main__':

  # Store the time we started
  TIME_START=time.strftime("%Y%m%d_%H%M")

  # Define command line interface
  parser = argparse.ArgumentParser(description='run_job')
  parser.add_argument('sin_file', action="store", help='Path to input.sin file')
  #Commented this line as the code now grabs num_runs from the .sin file
  #parser.add_argument('num_runs', action="store", help='number of runs')
  parser.add_argument('-l', '--length', action="store", choices=['s','m','l'], help='Specify s for a short job, m for a medium job or l for a long job')
# parser.add_argument('-a', '--append', action="store", choices=['n','a'], help='Specify whether to write a new file or append to the old one. n writes new file, a appends.')
  parser.add_argument('--debug', action="store_true", dest='debug', default=False, help='Enable debug output')
  parser.add_argument('--project_dir', action="store", dest='project_dir', default='/lustre/scratch/epp/ilc/$USER', help='Full path to where the output folder will be')
  #parser.add_argument('--log_dir', action="store", dest='log_dir', default='/lustre/scratch/epp/ilc/$USER/logs', help='Full path to where logs')
  parser.add_argument('--overwrite', action="store_true", dest='overwrite', default=False, help='If the log directory already exists allow it to be overwritten')
  args = parser.parse_args()

  #Core arguments. Santize the inputs somewhat by removing whitespace and trailing '/'
  sin_file=args.sin_file.strip().rstrip('/')
  sin_file=os.path.abspath(sin_file)
  project_dir=args.project_dir.strip().rstrip('/')

  #Set Mokka run length (if user enters a value that is not s/m/l, default to short)
  if args.length == 's':
    mokka_length = 'mps.short'
  elif args.length == 'm':
    mokka_length = 'mps.medium'
  elif args.length == 'l':
    mokka_length = 'mps.long'
  else:
    mokka_length = 'mps.short'

  # Expand OS variables
  project_dir=os.path.expandvars(project_dir)
  base_log_dir=project_dir+'/ilc_files/run_job/output'

  # Set up logging
  logger = logging.getLogger('run_job')
  if args.debug == True:
    logger.setLevel(logging.DEBUG)
  else:
    logger.setLevel(logging.INFO)

  #Pull the number of events out of the sindarin file
  with open(sin_file) as f: lines = f.read().splitlines()
  for line in lines:
    if line.startswith('n_events ='):
      line = line.split('#', 1)[0]
      line = line.rstrip()
      num_runs = int(line.split('=')[-1].strip())

  # Create console handler
  ch = logging.StreamHandler()
  # create log formater and add to handler(s)
  formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
  ch.setFormatter(formatter)
  # Add handlers to logger
  logger.addHandler(ch)

  # Checks to see whether the base_log_dir exists and if it doesn't, makes it. Should only be important the first time the script is run. 
  if os.path.isdir(base_log_dir) == False:
    os.makedirs(base_log_dir)
  else: 
    logger.warning('Outputs dir: {} already exists.'.format(base_log_dir))

  #EDIT - Defining basename (moved to make other things work)
  basename = os.path.splitext(os.path.basename(sin_file))[0] 

  # EDIT - creates event directory
  event_dir=base_log_dir+'/'+basename
  # Attempt to make log_dir
  if os.path.isdir(event_dir) == False:
    os.makedirs(event_dir)
  else: 
    logger.warning('Event dir: {} already exists.'.format(event_dir))

  # Create a log_dir in the same location as the destination directory,
  # relative to the log_dir argument given on the command line
  log_dir=event_dir+'/'+TIME_START+'-'+str(num_runs)+"events"
  # Attempt to make the log_dir
  try:
    os.makedirs(log_dir)
  except OSError:
    if args.overwrite == True and os.path.isdir(log_dir) == True:
      logger.warning('Log dir: {} exists. Overwriting...'.format(log_dir))
    else:
      raise

  # Set up a log file for script under the log directory
  fh = logging.FileHandler('{}/run_job.log'.format(log_dir))
  fh.setFormatter(formatter)
  logger.addHandler(fh)

  # Filenames of stdhep_file and slcio_file
  #basename = os.path.splitext(os.path.basename(sin_file))[0] 
  stdhep_file = log_dir+'/whizard/'+basename+'.stdhep'
  slcio_file = log_dir+'/mokka/'+basename+'.slcio'
  	
  # Log arguments
  logger.info('Project Dir:     {}'.format(project_dir))
  logger.info('Log Dir:         {}'.format(log_dir))
  logger.info('Sin File:        {}'.format(sin_file))
  logger.info('Num Runs:        {}'.format(num_runs))
  logger.info('Job Length:      {}'.format(mokka_length))

  # Copy input.sin file to log_dir
  subprocess.Popen('cp {} {}/'.format(sin_file, log_dir), shell=True, universal_newlines=True)

  # Generate stdhep.mac and clic01_ild.steer files
  macro_template = project_dir+'/ilc_files/run_job/run_job_files/stdhep.mac.template'
  macro_file = log_dir+'/stdhep.mac'
  steer_template = project_dir+'/ilc_files/run_job/run_job_files/clic01_ild.steer.template'
  steer_file = log_dir+'/clic01_ild.steer'

  with open(macro_file,'w') as macro:
    cmd="sed -e 's|<STDHEP_FILE>|'{}'|g' -e 's|<NUM_RUNS>|{}|g' {}".format(stdhep_file, num_runs, macro_template)
    proc = subprocess.Popen(cmd, shell=True, universal_newlines=True, stdout=macro)

  with open(steer_file,'w') as steer:
    cmd="sed -e 's|<MACRO_FILE>|'{}'|g' -e 's|<SLCIO_FILE>|{}|g' {}".format(macro_file, slcio_file, steer_template)
    proc = subprocess.Popen(cmd, shell=True, universal_newlines=True, stdout=steer)

# Alternative search-and-replace for the Mokka steering file to add an option to change write mode from "new" to "append" 
# with open(steer_file,'w') as steer:
#   cmd="sed -e 's|<MACRO_FILE>|'{}'|g' -e 's|<SLCIO_FILE>|{}|g' -e 's|<WRITE_MODE>|{}|g' {}".format(macro_file, slcio_file, write_mode, steer_template)
#   proc = subprocess.Popen(cmd, shell=True, universal_newlines=True, stdout=steer)

  logger.info('Running whizard...')
  whizard_job_name = 'whizard'
  whizard_command = 'cd {} && mkdir whizard && cd whizard && source /lustre/scratch/epp/ilc/GeneralILCsetup.sh; whizard {}'.format(log_dir, sin_file)
  whizard_output = log_dir+'/'+'whizard.job.log'
  whizard_job_id = submit_job(whizard_job_name, whizard_command, whizard_output, "mps.short")

  logger.info('Stdhep File:     {}'.format(stdhep_file))
  logger.info('Slcio File:      {}'.format(slcio_file))

  logger.info('Running Mokka...')
  mokka_job_name='mokka'
  mokka_command = 'cd {} && mkdir mokka && cd mokka && source /lustre/scratch/epp/ilc/GeneralILCsetup.sh; Mokka {}'.format(log_dir, steer_file)
  mokka_output = log_dir+'/'+'mokka.job.log'
  mokka_job_id = submit_dependent_job(mokka_job_name, mokka_command, mokka_output, whizard_job_id, mokka_length)

  logger.info('Running dumpevent...')
  dumpevent_job_name='dumpevent'
  dumpevent_command = 'source /lustre/scratch/epp/ilc/GeneralILCsetup.sh; dumpevent {} 1'.format(log_dir, slcio_file)
  dumpevent_output = log_dir+'/'+'dumpevent_{}.log'.format(basename)
  dumpevent_job_id = submit_dependent_job(dumpevent_job_name, dumpevent_command, dumpevent_output, mokka_job_id, "mps.short")
