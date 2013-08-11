#!/usr/bin/perl

@program_names = ("main");
%program_nodes = ("main", 15);

$program_to_run = $ARGV[0];
if (!$program_to_run || !$program_nodes{$program_to_run}) {
  die "Must enter program name to run. Possible programs are: " .
      "\n@program_names\n";
} else {
    $mpiexec = "mpiexec";
  }
  if ($ENV{"MPI_HOSTS"}) {
    $hosts = "-f " . $ENV{"MPI_HOSTS"};
  } else {
    $hosts = "hostfile";
  }

  print "$mpiexec -n $program_nodes{$program_to_run} -f $hosts ./$program_to_run\n";
  system("$mpiexec -n $program_nodes{$program_to_run} -f $hosts ./$program_to_run");

