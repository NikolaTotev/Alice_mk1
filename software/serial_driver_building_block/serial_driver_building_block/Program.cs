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
using Windows.Media.Core;
public struct RobotState
{
    public int joint_1_dir;
    public int joint_2_dir;
    public int joint_3_dir;
    public int joint_4_dir;
    public int joint_5_dir;
    public int joint_1_speed;
    public int joint_2_speed;
    public int joint_3_speed;
    public int servo_speed;
    public int stop;
    public int gripper_open;
}
internal class Program
{


    private static async Task Main(string[] args)
    {

        // Initialization code (same as before)
        var count = RawGameController.RawGameControllers.Count;
        count = RawGameController.RawGameControllers.Count;

        while (count == 0)
        {
            count = RawGameController.RawGameControllers.Count;
            Console.WriteLine("No controller found!");
            await Task.Delay(1000);

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
        bool previous_gripper_position = false;
        bool isStopped = true;
        DateTime lastGripperToggle = DateTime.MinValue;
        int GRIPPER_DEBOUNCE_MS = 250; 

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
            double mapped_1 = 1000;
            double mapped_2 = 1000;
            double mapped_3 = 1000;


            if (axes[0] >= 0.6)
            {
                mapped_2 = MapAxisValue(axes[0]);
            }
            else
            {
                mapped_2 = MapAxisValueLeft(axes[0]);
            }

            if (axes[1] >= 0.6)
            {
                mapped_3 = MapAxisValue(axes[1]);
            }
            else
            {
                mapped_3 = MapAxisValueLeft(axes[1]);
            }

            if (axes[2] >= 0.6)
            {
                mapped_1 = MapAxisValue(axes[2]);
            }
            else
            {
                mapped_1 = MapAxisValueLeft(axes[2]);
            }

            RobotState state = new RobotState();

            state.stop = Convert.ToInt32(buttons[0]);
            if (buttons[3])
            {
                DateTime now = DateTime.Now;
                if ((now - lastGripperToggle).TotalMilliseconds >= GRIPPER_DEBOUNCE_MS)
                {
                    state.gripper_open = Convert.ToInt32(!previous_gripper_position);
                    previous_gripper_position = Convert.ToBoolean(state.gripper_open);
                    lastGripperToggle = now;

                }
            }
            else
            {
                state.gripper_open = Convert.ToInt32(!previous_gripper_position);
            }


            //Joint 1 direction handling
            //CW = true CCW = false
            if (axes[2] >= 0.6)
            {
                state.joint_1_dir = Convert.ToInt32(false);
            }
            else
            {
                state.joint_1_dir = Convert.ToInt32(true);
            }

            //Joint 2 direction handling
            //CW = true CCW = false
            if (axes[0] >= 0.6)
            {
                state.joint_2_dir = Convert.ToInt32(true);
            }
            else
            {
                state.joint_2_dir = Convert.ToInt32(false);
            }

            //Joint 3 direction handling
            //CW = true CCW = false
            if (axes[1] >= 0.6)
            {
                state.joint_3_dir = Convert.ToInt32(false);
            }
            else
            {
                state.joint_3_dir = Convert.ToInt32(true);
            }

            //Joint 4 direction handling
            //CW = true CCW = false
            if (switches[0] == GameControllerSwitchPosition.Down)
            {
                state.joint_4_dir = Convert.ToInt32(false);
                state.servo_speed = 1;

            }

            if (switches[0] == GameControllerSwitchPosition.Up)
            {
                state.joint_4_dir = Convert.ToInt32(true);
                state.servo_speed = 2;

            }

            if (switches[0] == GameControllerSwitchPosition.Right)
            {
                state.joint_5_dir = Convert.ToInt32(true);
                state.servo_speed = 3;

            }

            if (switches[0] == GameControllerSwitchPosition.Left)
            {
                state.joint_5_dir = Convert.ToInt32(false);
                state.servo_speed = 4;

            }

            if (switches[0] == GameControllerSwitchPosition.Center)
            {
                state.servo_speed = 0;
            }             

            if (mapped_1 < 300 && mapped_1 != 0)
            {
                mapped_1 = 300;
            }

            if (mapped_3 < 200 && mapped_3 != 0)
            {
                mapped_3 = 200;
            }

            if (mapped_2 < 400 && mapped_2 != 0)
            {
                mapped_2 = 400;
            }


            state.joint_1_speed = Convert.ToInt32(mapped_1);
            state.joint_2_speed = Convert.ToInt32(mapped_2);
            state.joint_3_speed = Convert.ToInt32(mapped_3);


            await communicator.SendCustomCommand($"us_{state.stop}_" +
                $"{state.joint_1_dir}_" +
                $"{state.joint_2_dir}_" +
                $"{state.joint_3_dir}_" +
                $"{state.joint_4_dir}_" +
                $"{state.joint_5_dir}_" +
                $"{state.joint_1_speed}_" +
                $"{state.joint_2_speed}_" +
                $"{state.joint_3_speed}_" +
                $"{state.servo_speed}_" +
                $"{state.gripper_open}");

            Console.WriteLine($"SENT STATE IS: {state.stop}_" +
                $"{state.joint_1_dir}_" +
                $"{state.joint_2_dir}_" +
                $"{state.joint_3_dir}_" +
                $"{state.joint_4_dir}_" +
                $"{state.joint_5_dir}_" +
                $"{state.joint_1_speed}_" +
                $"{state.joint_2_speed}_" +
                $"{state.joint_3_speed}_" +
                $"{state.servo_speed}_" +
                $"{state.gripper_open}");

            // Small delay to prevent overwhelming the serial port
            await Task.Delay(42);
        }
    }
    static int BoolToInt(bool value)
    {
        return value ? 1 : 0;
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
        Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] << {data}");
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