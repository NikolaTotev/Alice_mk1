//internal class Program
//{
//    // Method 1: Async Main (C# 7.1+) - RECOMMENDED
//    private static async Task Main(string[] args)
//    {
//        Console.WriteLine("=== RP2040 Serial Communication Example ===\n");

//        var communicator = new RP2040SerialCommunicator();             

//        // Show available ports
//        Console.WriteLine("Available COM ports:");
//        foreach (string port in RP2040SerialCommunicator.GetAvailablePorts())
//        {
//            Console.WriteLine($"  {port}");
//        }

//        Console.WriteLine("Select port to use...");
//        string comPort = Console.ReadLine();

//        // Connect to the Pico (adjust COM port as needed)
//        //string comPort = "COM3"; // Change this to your Pico's COM port
//        Console.WriteLine($"\nAttempting to connect to {comPort}...");

//        if (communicator.Connect(comPort))
//        {
//            Console.WriteLine("Connected successfully!\n");

//            // Example operations with proper async/await
//            await Task.Delay(1000); // Give time for connection to stabilize
//            Console.WriteLine("Sending start command");
//            await communicator.SendCustomCommand("start_cli>");
//            await Task.Delay(1000); // Give time for connection to stabilize


//            // Try to read response asynchronously
//            Console.WriteLine("\nAttempting to read response...");
//            string response = await communicator.ReadDataAsync(2000);
//            if (response != null)
//            {
//                Console.WriteLine($"Synchronous read: {response}");
//            }
//            else
//            {
//                Console.WriteLine("No response received within timeout");
//            }


//            // Keep the program running to receive data
//            //Console.WriteLine("\n=== Communication established. Monitoring for data... ===");
//            //Console.WriteLine("Press any key to exit...\n");
//            while(true)
//            {

//            }
//            Console.ReadKey();

//            communicator.Disconnect();
//        }
//        else
//        {
//            Console.WriteLine($"Failed to connect to {comPort}");
//        }

//        communicator.Dispose();
//        Console.WriteLine("Application closed.");
//    }
//}


using System.Net.WebSockets;
using Windows.Gaming.Input;
internal class Program
{
    private static async Task Main(string[] args)
    {

        // Initialization code (same as before)
        var count = RawGameController.RawGameControllers.Count;
        count = RawGameController.RawGameControllers.Count;

        if (count == 0)
        {
            Console.WriteLine("No controller found!");
            return;
        }

        var js = RawGameController.RawGameControllers[0];

        Console.Clear();
        Console.WriteLine($"Controller: {js.DisplayName ?? "Unknown"}\n");


        Console.WriteLine("=== RP2040 Serial Communication Example ===\n");

        var communicator = new RP2040SerialCommunicator();

        // Subscribe to events BEFORE connecting
        communicator.DataReceived += OnDataReceived;
        communicator.ErrorOccurred += OnErrorOccurred;
        communicator.ModeChanged += OnModeChanged;

        // Show available ports
        Console.WriteLine("Available COM ports:");
        foreach (string port in RP2040SerialCommunicator.GetAvailablePorts())
        {
            Console.WriteLine($"  {port}");
        }

        Console.WriteLine("Select port to use...");
        string comPort = Console.ReadLine();

        Console.WriteLine($"\nAttempting to connect to {comPort}...");

        if (communicator.Connect(comPort))
        {
            Console.WriteLine("Connected successfully!\n");

            // Give time for connection to stabilize
            await Task.Delay(1000);

            Console.WriteLine("Sending start command");
            await communicator.SendCustomCommand("start_cli>");

            Console.WriteLine("\n=== Communication established. Monitoring for data... ===");
            Console.WriteLine("Commands available:");
            Console.WriteLine("  'q' or 'quit' - Exit application");
            Console.WriteLine("  'data:<message>' - Send data");
            Console.WriteLine("  'get' - Request data");
            Console.WriteLine("  Any other text will be sent as custom command\n");

            // Interactive loop to handle user input while receiving data
            await InteractiveLoop(communicator, js);

            communicator.Disconnect();
        }
        else
        {
            Console.WriteLine($"Failed to connect to {comPort}");
        }

        communicator.Dispose();
        Console.WriteLine("Application closed.");
    }

    private static async Task InteractiveLoop(RP2040SerialCommunicator communicator, RawGameController js)
    {
        Console.WriteLine("Staring loop");
        var axes = new double[js.AxisCount];
        var switches = new GameControllerSwitchPosition[js.SwitchCount];
        var buttons = new bool[js.ButtonCount];
        var previousHatPosition = GameControllerSwitchPosition.Center;
        bool isStopped = true;

        while (true)
        {
            //Console.Write("> Joystick ctrl");
            string input = "";//Console.ReadLine();

            //if (string.IsNullOrEmpty(input))
            //    continue;

            // Handle exit commands
            if (input.ToLower() == "q" || input.ToLower() == "quit")
            {
                break;
            }

            js.GetCurrentReading(buttons, switches, axes);
            //Console.WriteLine($"{switches.Length}");
            double mapped = 1000;

            if (axes[0] >= 0.6)
            {
                mapped = MapAxisValue(axes[0]);
            }
            else
            {
                mapped = MapAxisValueLeft(axes[0]);
            }
            
                        
            
            if (buttons[0])
            {
                if (mapped == 0)
                {
                    if(!isStopped)
                    {
                        await communicator.SendCustomCommand("stop");
                        Console.WriteLine("Stopping");
                        isStopped = true;
                    }                    
                }

                if (axes[0] >= 0.6)
                {
                    if (isStopped)
                    {
                        await communicator.SendCustomCommand($"mcw_{Convert.ToInt32(mapped)}");
                        Console.WriteLine($"mcw_{Convert.ToInt32(mapped)}");
                        isStopped = false;
                    }
                    else
                    {
                        await communicator.SendCustomCommand($"update-speed_{Convert.ToInt32(mapped)}");
                        Console.WriteLine($"mcw_{Convert.ToInt32(mapped)}");
                    }

                }

                if (axes[0] <= 0.4)
                {
                    if (isStopped)
                    {
                        await communicator.SendCustomCommand($"mccw_{Convert.ToInt32(mapped)}");
                        Console.WriteLine($"mccw_{Convert.ToInt32(mapped)}");
                        isStopped = false;
                    }

                    else
                    {
                        await communicator.SendCustomCommand($"update-speed_{Convert.ToInt32(mapped)}");
                        Console.WriteLine($"update-speed_{Convert.ToInt32(mapped)}");
                    }
                }


                //if (switches.Length > 0)
                //{
                //    var currentHatPosition = switches[0];

                //    if (currentHatPosition != previousHatPosition)
                //    {

                //        if (switches[0] == GameControllerSwitchPosition.Center)
                //        {
                //            await communicator.SendCustomCommand("stop");
                //            Console.WriteLine("Stopping");
                //            isStopped = true;
                //        }

                //        if (switches[0] == GameControllerSwitchPosition.Right)
                //        {
                //            await communicator.SendCustomCommand("mcw_800");
                //            Console.WriteLine("mcw");
                //            isStopped = false;
                //        }

                //        if (switches[0] == GameControllerSwitchPosition.Left)
                //        {
                //            await communicator.SendCustomCommand("mccw");
                //            Console.WriteLine("mccw");
                //            isStopped = false;
                //        }

                //        previousHatPosition = switches[0];
                //    }
                //}
            }
            else
            {
                if (!isStopped)
                {
                    await communicator.SendCustomCommand("stop");
                    Console.WriteLine("Stopping");
                    isStopped = true;
                }

            }






            //await communicator.SendCustomCommand(input);


            // Small delay to prevent overwhelming the serial port
            await Task.Delay(50);
        }
    }

    static double MapAxisValue(double rawValue)
    {
        return rawValue < 0.6 ? 0 : 1000 - ((rawValue - 0.6) / 0.4 * 950);
    }

    static double MapAxisValueLeft(double rawValue)
    {
        return rawValue > 0.4 ? 0 : 50 + (rawValue / 0.4 * 1000);
    }
    // Event handlers for continuous data reception
    private static void OnDataReceived(object sender, string data)
    {
        Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] << {data}\n");
    }

    private static void OnErrorOccurred(object sender, string error)
    {
        Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] ERROR: {error}");
    }

    private static void OnModeChanged(object sender, OperationalMode mode)
    {
        Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] MODE CHANGED: {mode}");
    }
}