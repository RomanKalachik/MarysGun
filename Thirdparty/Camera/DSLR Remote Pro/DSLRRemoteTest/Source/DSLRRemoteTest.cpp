// DSLRRemoteTest.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <time.h>
#include <sys/stat.h>
#include "DSLRRemoteLib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

using namespace std;

const char* TimeStr()
{
	static char str[256];
	time_t t = time(0);
	struct tm now;
	localtime_s(&now, &t);
	strftime(str, sizeof(str), "%H:%M:%S", &now);
	return str;
}

void run_webcam(int shots, int interval)
{
	bool error = false;

	cout << "Starting webcam sequence of " << shots << " shots with " << interval << " secs between shots" << endl;
	_tzset();

	char model[256];
	if (::GetCameraModel(model, sizeof(model)) == 0)
	{
		cout << "Camera model: " << model << endl;
	}

	int i = 0;
	while (i++ < shots && !error)
	{

		cout << (const char*)TimeStr() << ": Taking shot number: " << i << endl;
		time_t nextShot = time(0) + interval;

		char filename[1024];
		int status = ::ReleaseShutter(60, filename, sizeof(filename));
		if (status == 0)
		{
			if (filename && strlen(filename) > 0)
			{
				cout << (const char*)TimeStr() << ": Image saved as: " << filename << endl;
				struct _stat status;
				if (_stat("webcam.jpg", &status) == 0)
				{
					if (_unlink("webcam.jpg") != 0)
					{
						cerr << "Error renaming file" << endl;
						error = true;
					}
				}
				if (rename(filename, "webcam.jpg") == 0)
				{
					cout << (const char*)TimeStr() << ": Image renamed 'webcam.jpg'" << endl;
				}
				else
				{
					cerr << "Error renaming file" << endl;
					error = true;
				}
			}
			else
			{
				cout << (const char*)TimeStr() << ": Image saved to CF card only?" << endl;
			}

			// wait for next shot
			time_t now = time(0);
			if (!error && now < nextShot && i < shots)
			{
				long secs = (long)(nextShot - now);
				cout << (const char*)TimeStr() << ": Sleeping for " << secs << " secs before next shot" << endl;
				_sleep(secs * 1000);
			}
		}
		else
		{
			error = true;
			switch(status)
			{
			case 1:
				cerr << "DSLR Remote Pro is not running" << endl;
				break;
			case 2:
				cerr << "Camera is not connected" << endl;
				break;
			case 3:
				cerr << "Camera is busy" << endl;
				break;
			case 4:
				cerr << "Timeout waiting for image to be saved" << endl;
				break;
			case 5:
				cerr << "Error releasing shutter e.g. AF failure" << endl;
				break;
			default:
				cerr << "ERROR: unexpected return status: " << status << endl;
			}
		}
	}
}

void Usage()
{
	cerr << "Usage: DSLRRemoteTest <options>" << endl;
	cerr << "\t-h              print this usage information" << endl;
	cerr << "\t-w <shots>      run camera as a webcam for <shots> shots" << endl;
	cerr << "\t                images are saved as webcam.jpg in the current directory" << endl;
	cerr << "\t-t <interval>   specify the number of seconds between shots when" << endl;
	cerr << "\t                used as a webcam" << endl;
	cerr << "\t-S <camera#>    select camera (multi-camera versions of DSLR Remote Pro only)" << endl;
	cerr << "\t-C <connect>    connect or disconnect from the camera" << endl;
	cerr << "\t-a <aperture>   set the aperture, 0 = widest aperture" << endl;
	cerr << "\t-b <WB>         set the white balance or kelvin color temp" << endl;
	cerr << "\t-e <mode>       set the exposure mode (1D series cameras only)" << endl;
	cerr << "\t-s <shutter>    set the shutter speed, 0 = longest shutter speed" << endl;
	cerr << "\t-x <comp>       set the exposure compensation" << endl;
	cerr << "\t-i <quality>    set the image size and quality" << endl;
	cerr << "\t-I <ISO>        set the ISO" << endl;
	cerr << "\t-L <enable>     display/hide live view window" << endl;
	cerr << "\t-E <pathname>   capture live view frame to JPEG file" << endl;
	cerr << "\t-f <step>       focus lens: 1=near3, 2=near2, 3=near1, 4=far1, 5=far2, 6=far3" << endl;
	cerr << "\t-c <comment>    set comment to be added to images (max 255 chars)" << endl;
	cerr << "\t-p <prefix>     set the filename prefix (max 255 chars)" << endl;
	cerr << "\t-o <directory>  set the output directory" << endl;
	cerr << "\t-B <duration>   press shutter button for <duration> tenths of a sec" << endl;
	cerr << "\t-q              query output directory" << endl;
	cerr << "\t-n              don't release shutter" << endl;
	cerr << "\t-X              exit DSLR Remote Pro" << endl;
	exit(0);
}

int main(int argc, const char* argv[])
{
	int nRetCode = 0;

	int webcam_interval = 30;
	int webcam_shots = 10;
	bool webcam = false;
	bool releaseShutter = true;
	int shutterButtonDuration = 0;

	// parse the command line arguments
	int arg = 1;
	while (arg < argc)
	{
		if (strcmp(argv[arg], "-h") == 0)
		{
			Usage();
		}
		else if (strcmp(argv[arg], "-w") == 0)
		{
			webcam = true;
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					webcam_shots = atoi(argv[arg]);
					arg++;
				}
			}
		}
		else if (strcmp(argv[arg], "-B") == 0)
		{
			// Press shutter release for given number of tenths of a second
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					shutterButtonDuration = atoi(argv[arg]);
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-t") == 0)
		{
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					webcam_interval = atoi(argv[arg]);
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-C") == 0)
		{
			// connect/disconnect
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					int result = ::ConnectDisconnect(atoi(argv[arg]) ? true : false, 60);
					if (result)
					{
						cerr << "Error connecting/disconnecting, error: " << result << endl;
						exit(1);
					}
					arg++;
				}
				else
				{
					Usage();
				}
				releaseShutter = false;
			}
		}
		else if (strcmp(argv[arg], "-S") == 0)
		{
			// select camera
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					int result = ::SelectCamera(atoi(argv[arg]));
					if (result)
					{
						cerr << "Error selecting camera, error: " << result << endl;
						exit(1);
					}
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-S") == 0)
		{
			// select camera
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					int result = ::SelectCamera(atoi(argv[arg]));
					if (result)
					{
						cerr << "Error selecting camera, error: " << result << endl;
						exit(1);
					}
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-s") == 0)
		{
			// shutter speed
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					int result = ::SetShutterAperture(atoi(argv[arg]), -1);
					if (result)
					{
						cerr << "Error setting shutter speed, error: " << result << endl;
						exit(1);
					}
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-a") == 0)
		{
			// aperture
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					int result = ::SetShutterAperture(-1, atoi(argv[arg]));
					if (result)
					{
						cerr << "Error setting aperture, error: " << result << endl;
						exit(1);
					}
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-I") == 0)
		{
			// ISO
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					int result = ::SetISO(atoi(argv[arg]));
					if (result)
					{
						cerr << "Error setting ISO, error: " << result << endl;
						exit(1);
					}
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-L") == 0)
		{
			// show/hide live view window
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					int result = ::DisplayEVF(atoi(argv[arg]) != 0);
					if (result)
					{
						cerr << "Error setting EVF, error: " << result << endl;
						exit(1);
					}
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-f") == 0)
		{
			// focus lens when EVF is displayed
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					int focus = atoi(argv[arg]);
					if (focus >= 1 && focus <= 6)
					{
						int result = ::SetLensFocus(focus);
						if (result)
						{
							cerr << "Error focusing lens, error: " << result << endl;
							exit(1);
						}
					}
					else
					{
						cerr << "Error lens focus step must be in the range 1 to 6" << endl;
					}
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-E") == 0)
		{
			// capture live view frame to file
			arg++;
			if (arg < argc)
			{
				const char* p = argv[arg];
				int result = ::SaveEvfFrame(argv[arg]);
				if (result)
				{
					cerr << "Error saving live view frame, error: " << result << endl;
					exit(1);
				}
				arg++;
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-e") == 0)
		{
			// exposure mode
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					int result = ::SetExposureMode(atoi(argv[arg]));
					if (result)
					{
						cerr << "Error setting exposure mode, error: " << result << endl;
						exit(1);
					}
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-i") == 0)
		{
			// Image size and quality
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					int result = ::SetImageSizeQuality(atoi(argv[arg]));
					if (result)
					{
						cerr << "Error setting image size and quality, error: " << result << endl;
						exit(1);
					}
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-x") == 0)
		{
			// exposure compensation
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					int result = ::SetExposureCompensation(atoi(argv[arg]));
					if (result)
					{
						cerr << "Error setting exposure compensation, error: " << result << endl;
						exit(1);
					}
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-b") == 0)
		{
			// white balance
			arg++;
			if (arg < argc)
			{
				if (isdigit(argv[arg][0]))
				{
					int result = ::SetWhiteBalance(atoi(argv[arg]));
					if (result)
					{
						cerr << "Error setting white balance, error: " << result << endl;
						exit(1);
					}
					arg++;
				}
				else
				{
					Usage();
				}
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-c") == 0)
		{
			// comment
			arg++;
			if (arg < argc)
			{
				int result = ::SetComment(argv[arg]);
				if (result)
				{
					cerr << "Error setting comment, error: " << result << endl;
					exit(1);
				}
				arg++;
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-p") == 0)
		{
			// filename prefix
			arg++;
			if (arg < argc)
			{
				int result = ::SetFilenamePrefix(argv[arg]);
				if (result)
				{
					cerr << "Error setting filename prefix, error: " << result << endl;
					exit(1);
				}
				arg++;
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-X") == 0)
		{
			// Exit DSLR Remote Pro
			arg++;
			releaseShutter = false;
			int result = ::ExitApp();
			if (result)
			{
				cerr << "Error exiting DSLR Remote Pro, error: " << result << endl;
				exit(1);
			}
		}
		else if (strcmp(argv[arg], "-o") == 0)
		{
			// output directory
			arg++;
			if (arg < argc)
			{
				int result = ::SetOutputPath(argv[arg]);
				if (result)
				{
					cerr << "Error setting output directory, error: " << result << endl;
					exit(1);
				}
				arg++;
			}
			else 
			{
				Usage();
			}
		}
		else if (strcmp(argv[arg], "-q") == 0)
		{
			// query output directory
			arg++;
			char pathname[256];
			int result = ::GetOutputPath(pathname, sizeof(pathname));
			if (result)
			{
				cerr << "Error querying output directory, error: " << result << endl;
				exit(1);
			}
			else
			{
				cout << "Output directory: " << pathname << endl;
			}
		}
		else if (strcmp(argv[arg], "-n") == 0)
		{
			// don't release shutter
			arg++;
			releaseShutter = false;
		}
		else
		{
			Usage();
		}
	}

	if (webcam)
	{
		run_webcam(webcam_shots, webcam_interval);
	}
	else if (releaseShutter)
	{
		// take a single shot and report the filename/status
		char filename[256];
		int status;
		if (shutterButtonDuration > 0)
		{
			// press and hold shutter button for shutterButtonDuration tenths of a second. This may result in several photos if
			// the camera is set to continuous shooting.
			status = ShutterButton(shutterButtonDuration, 60 + 2 * shutterButtonDuration, filename, sizeof(filename));
		}
		else
		{
			// press and release shutter button to take photo
			status = ReleaseShutter(60, filename, sizeof(filename));
		}
		switch(status)
		{
		case 0: // success
			if (filename && strlen(filename))
			{
				cout << "Success, image saved as: " << filename << endl;
			}
			else
			{
				cout << "Success, image saved on CF card?" << endl;
			}
			break;

		case 1:
			cerr << "DSLRRemote is not running" << endl;
			break;
		case 2:
			cerr << "Camera is not connected" << endl;
			break;
		case 3:
			cerr << "Camera is busy" << endl;
			break;
		case 4:
			cerr << "Timeout waiting for image to be saved" << endl;
			break;
		case 5:
			cerr << "Error releasing shutter e.g. AF failure" << endl;
			break;
		default:
			cerr << "ERROR: unexpected return status: " << status << endl;
		}
	}

	return nRetCode;
}



