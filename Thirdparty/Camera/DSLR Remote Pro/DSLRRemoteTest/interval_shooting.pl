# Perl script for running a time-lapse sequence with DSLR Remote Pro using
# the example command line app DSLRRemoteTest.exe
#
# This script comes with no warranty or support whatsoever and may
# be freely copied or modified as required.
#
# Perl for Windows can be downloaded from http://www.activestate.com

$installDir = "C:\\Program Files\\BreezeSys\\DSLR Remote Pro";
$cmd = "\"$installDir\\DSLRRemoteTest\\DSLRRemoteTest.exe\"";
$dslremote = "$installDir\\DSLRRemote.exe";
$spawn = "\"$installDir\\DSLRRemoteTest\\SpawnGuiApp.exe\" $dslremote";

`cd $installDir`;
$cameraStartupTime = 3;	# approx time in secs it takes to connect to camera and take photo

# take pictures between 08:00 and 18:00
$startHour = 8;
$endHour = 18;
$intervalInMins = 1;	# interval between shots (in minutes)
$bracket = 0;		# set this to 1 to bracket the shots +/- one stop or 0 to take a single shot
$disconnect = 0;	# set this to 1 to disconnect from the camera after taking each photo

$startTimeInSecs = 3600 * $startHour;
$endTimeInSecs = 3600 * $endHour;
$secsInDay = 3600 * 24;
$intervalInSecs = 60 * $intervalInMins;
LogMsg("Script started");
LogMsg("Running DSLR Remote Pro");
RunDSLRRemote();
while (1)
{
	my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
	my $waitingTime;

	my $secsIntoDay = $sec + 60 * ($min + 60 * $hour);
	if ($secsIntoDay < $startTimeInSecs)
	{
		# wait until start hour
		$waitingTime = $startTimeInSecs - $secsIntoDay;
	}
	elsif ($secsIntoDay > $endTimeInSecs)
	{
		# after end hour, wait until start hour tomorrow
		$waitingTime = $secsInDay + $startTimeInSecs - $secsIntoDay;
	}
	else
	{
		$waitingTime = $intervalInSecs - ($secsIntoDay % $intervalInSecs);
	}
	if ($disconnect)
	{
		$waitingTime -= $cameraStartupTime;
	}

	if ($waitingTime > 0)
	{
		my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time + $waitingTime);
		my $msg = sprintf("Waiting for $waitingTime secs... (until %02d/%02d/%02d %02d:%02d:%02d)", $year + 1900, $mon + 1, $mday, $hour, $min, $sec);
		LogMsg($msg);
		`sleep $waitingTime`;
		if (!$disconnect)
		{
			TakePhotoAndOptionallyDisconnect();
		}
	}
	else
	{
		TakePhotoAndOptionallyDisconnect();
		if ($disconnect)
		{
			`sleep $cameraStartupTime`;
		}
		`sleep 1`;
	}
}
LogMsg("Closing DSLR Remote Pro");
ExitDSLRRemote();
LogMsg("Script exiting");

sub TakePhotoAndOptionallyDisconnect
{
	if ($disconnect)
	{
		LogMsg("Connecting...");
	}
	ConnectToCamera();

	if ($bracket)
	{
		LogMsg("Connected to camera, taking 3 shot auto-bracket sequence...");
		my $files = AutoBracket3();
		LogMsg("Bracketed images:\n$files");
	}
	else
	{
		LogMsg("Connected to camera, taking picture...");
		my $file = TakePhoto();
		LogMsg("Image: $file");
	}
	if ($disconnect)
	{
		DisconnectFromCamera();
		LogMsg("Disconnected from camera");
	}
}

sub LogMsg
{
	my ($msg) = (@_);
	my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
	printf("%02d/%02d/%02d %02d:%02d:%02d $msg\n", $year + 1900, $mon + 1, $mday, $hour, $min, $sec);
}

# Take a photo and return it's pathname if successful
sub TakePhoto
{
	my $result = `$cmd`;
	my $file;
	chomp $result;
	if ($result =~ /image saved as:\s+(.*)/)
	{
		$file = $1;
	}
	else
	{
		die "Error taking picture: $result\n";
	}
	return $file;
}

# Take a three shot autobracket sequence and return pathnames if successful
sub AutoBracket3
{
	my $files;
	my $result = `$cmd -x 6`;	# no exposure compensation
	if ($result =~ /image saved as:\s+(.*)/)
	{
		$files = $1;
	}
	else
	{
		die "Error taking picture: $result";
	}

	$result = `$cmd -x 3`;	# +1 stop compensation
	if ($result =~ /image saved as:\s+(.*)/)
	{
		$files .= "\n$1";
	}
	else
	{
		die "Error taking picture: $result";
	}

	$result = `$cmd -x 9`;	# -1 stop compensation
	if ($result =~ /image saved as:\s+(.*)/)
	{
		$files .= "\n$1";
	}
	else
	{
		die "Error taking picture: $result";
	}
	`$cmd -n -x 6`;	# set camera back to no exposure compensation
	return $files;
}

# Connect to camera and return camera model if successful
sub ConnectToCamera
{
	my $result = `$cmd -n -q 2>&1`;
	if (!($result =~ /error/i))
	{
		$result = `$cmd -n -C 1 2>&1`;
		chomp $result;
	}

	if ($result ne "")
	{
		# connection failed, try restarting DSLR Remote Pro
		LogMsg("Connection failed, restarting DSLR Remote Pro...");
		LogMsg("Exit DSLR Remote Pro...");
		`$cmd -n -X`;			# first exit
		LogMsg("Run DSLR Remote Pro...");
		system($spawn);			# then run
		`sleep $cameraStartupTime`;	# wait a while so that camera has time to connect
		`sleep 2`;

		# now see if we are connected
		LogMsg("Check connection...");
		$result = `$cmd -n -C 1 2>&1`;
		chomp $result;
		if ($result ne "")
		{
			LogMsg("Unable to connect to camera");
			die "Error connecting to camera: $result\n";
		}
	}
}

sub DisconnectFromCamera
{
	my $result = `$cmd -n -C 0 2>&1`;

	chomp $result;
	if ($result ne "")
	{
		die "Error disconnecting from camera: $result\n";
	}
}

sub RunDSLRRemote
{
	my $result = `$cmd -n -q 2>&1`;
	if ($result =~ /error/i)
	{
		# it doesn't look as though DSLR Remote Pro is running
		ExitDSLRRemote();	# exit DSLR Remote Pro just in case, then run it
		system($spawn);
	}
}

sub ExitDSLRRemote
{
	my $result = `$cmd -n -X 2>&1`;
	chomp $result;
	return $result;
}
