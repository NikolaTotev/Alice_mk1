//using System;
//using Windows.Gaming.Input;

//class Program
//{
//    static void Main()
//    {
//        while (true)
//        {
//            if (RawGameController.RawGameControllers.Count > 0)
//            {
//                var js = RawGameController.RawGameControllers[0];
//                var axes = new double[js.AxisCount];
//                var buttons = new bool[js.ButtonCount];

//                js.GetCurrentReading(buttons, null, axes);

//                Console.SetCursorPosition(0, 0);
//                Console.WriteLine($"X: {(axes.Length > 0 ? axes[0] : 0):F2}  Y: {(axes.Length > 1 ? axes[1]:0):F2}");

//                for (int i = 0; i < buttons.Length; i++)
//                    if (buttons[i]) Console.Write($"Button{i} ");
//                Console.WriteLine("          ");
//            }
//            System.Threading.Thread.Sleep(50);
//        }
//    }
//}
//=======================================================================
//using System;
//using Windows.Gaming.Input;

//class Program
//{
//    static void Main()
//    {
//        Console.WriteLine("Checking all controller types...\n");

//        int controller_count = RawGameController.RawGameControllers.Count;
//        controller_count = RawGameController.RawGameControllers.Count;

//        //Console.WriteLine($"Gamepads: {Gamepad.Gamepads.Count}");
//        //Console.WriteLine($"FlightSticks: {FlightStick.FlightSticks.Count}");
//        //Console.WriteLine($"RacingWheels: {RacingWheel.RacingWheels.Count}");
//        Console.WriteLine($"RawGameControllers: {controller_count}");

//        if (RawGameController.RawGameControllers.Count > 0)
//        {
//            foreach (var controller in RawGameController.RawGameControllers)
//            {
//                Console.WriteLine($"\nFound: {controller.DisplayName ?? "Unknown"}");
//                Console.WriteLine($"Hardware ID: {controller.HardwareProductId}");
//                Console.WriteLine($"Vendor ID: {controller.HardwareVendorId}");
//                Console.WriteLine($"Axis Count: {controller.AxisCount}");
//                Console.WriteLine($"Btn Count: {controller.ButtonCount}");

//            }
//        }


//        while(true)
//        {

//            Thread.Sleep(20);
//        }        
//    }
//}


//using System;
//using System.Threading;
//using Windows.Gaming.Input;

//class Program
//{
//    static void Main()
//    {
//        Console.WriteLine("Checking all controller types...\n");

//        int controller_count = RawGameController.RawGameControllers.Count;
//        controller_count = RawGameController.RawGameControllers.Count;

//        Console.WriteLine($"RawGameControllers: {controller_count}");

//        if (RawGameController.RawGameControllers.Count > 0)
//        {
//            foreach (var controller in RawGameController.RawGameControllers)
//            {
//                Console.WriteLine($"\nFound: {controller.DisplayName ?? "Unknown"}");
//                Console.WriteLine($"Hardware ID: {controller.HardwareProductId}");
//                Console.WriteLine($"Vendor ID: {controller.HardwareVendorId}");
//                Console.WriteLine($"Axis Count: {controller.AxisCount}");
//                Console.WriteLine($"Btn Count: {controller.ButtonCount}");
//            }
//        }

//        Console.WriteLine("\n\nStarting to poll controller...\n");

//        // Get the first controller
//        var js = RawGameController.RawGameControllers[0];

//        // Create arrays for reading based on the controller's capabilities
//        var axes = new double[js.AxisCount];
//        var buttons = new bool[js.ButtonCount];
//        var switches = new GameControllerSwitchPosition[js.SwitchCount];

//        // Store the starting cursor position for the polling output
//        int startLine = Console.CursorTop;

//        while (true)
//        {
//            // Read the current state
//            js.GetCurrentReading(buttons, switches, axes);

//            // Move cursor to consistent position for updating
//            Console.SetCursorPosition(0, startLine);

//            // Display axes
//            Console.WriteLine("AXES:                                    ");
//            for (int i = 0; i < axes.Length; i++)
//            {
//                // Show both raw (0-1) and centered (-1 to 1) values
//                double centered = (axes[i] * 2.0) - 1.0;
//                Console.WriteLine($"  Axis {i}: {axes[i]:F3} (centered: {centered,7:F3})");
//            }

//            // Display pressed buttons
//            Console.Write("\nBUTTONS PRESSED: ");
//            bool anyPressed = false;
//            for (int i = 0; i < buttons.Length; i++)
//            {
//                if (buttons[i])
//                {
//                    Console.Write($"[{i}] ");
//                    anyPressed = true;
//                }
//            }
//            if (!anyPressed)
//            {
//                Console.Write("None");
//            }
//            Console.WriteLine("                              "); // Clear the rest of the line

//            // Display POV/Hat if available
//            if (switches.Length > 0)
//            {
//                Console.WriteLine($"\nPOV/HAT: {switches[0],-15}");
//            }

//            Thread.Sleep(20);
//        }
//    }
//}


using System;
using System.Threading;
using Windows.Gaming.Input;

class Program
{
    static void Main()
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
        var axes = new double[js.AxisCount];
        var buttons = new bool[js.ButtonCount];
        var switches = new GameControllerSwitchPosition[js.SwitchCount];


        Console.Clear();
        Console.WriteLine($"Controller: {js.DisplayName ?? "Unknown"}  {count}, {js.AxisCount}, {js.ButtonCount}\n");
        //Console.WriteLine($"Counts: {count}, {js.AxisCount}, {js.ButtonCount}");

        while (true)
        {
            js.GetCurrentReading(buttons, switches, axes);
            Console.SetCursorPosition(0, 2);

            // Display mapped axes
            Console.WriteLine("MAPPED AXES:");
            for (int i = 0; i < axes.Length; i++)
            {
                double mapped = axes[i];// MapAxisValue(axes[i]);
                Console.WriteLine($"  Axis {i}: {mapped,4:F0} (raw: {axes[i]:F3})");
            }

            // Buttons
            Console.Write("\nButtons: ");
            for (int i = 0; i < buttons.Length; i++)
            {
                if (buttons[i]) Console.Write($"[{i}] ");
            }
            Console.WriteLine($"{js.SwitchCount} {switches[0]}");
            Console.WriteLine("        ");

            Thread.Sleep(20);
        }
    }

    static double MapAxisValue(double rawValue)
    {
        return rawValue < 0.6 ? 0 : 250 + ((rawValue - 0.6) / 0.4 * 350);
    }


}


//using System;
//using System.Runtime.InteropServices;
//using System.Threading;

//class Program
//{
//    [DllImport("xinput1_4.dll")]
//    static extern int XInputGetState(int dwUserIndex, ref XInputState pState);

//    [DllImport("kernel32.dll", SetLastError = true)]
//    static extern IntPtr GetStdHandle(int nStdHandle);

//    [DllImport("kernel32.dll")]
//    static extern bool GetConsoleMode(IntPtr hConsoleHandle, out uint lpMode);

//    [DllImport("kernel32.dll")]
//    static extern bool SetConsoleMode(IntPtr hConsoleHandle, uint dwMode);

//    [StructLayout(LayoutKind.Sequential)]
//    public struct XInputState
//    {
//        public uint dwPacketNumber;
//        public XInputGamepad Gamepad;
//    }

//    [StructLayout(LayoutKind.Sequential)]
//    public struct XInputGamepad
//    {
//        public ushort wButtons;
//        public byte bLeftTrigger;
//        public byte bRightTrigger;
//        public short sThumbLX;
//        public short sThumbLY;
//        public short sThumbRX;
//        public short sThumbRY;
//    }

//    const int STD_INPUT_HANDLE = -10;
//    const uint ENABLE_QUICK_EDIT = 0x0040;
//    const uint ENABLE_MOUSE_INPUT = 0x0010;
//    const uint ENABLE_WINDOW_INPUT = 0x0008;

//    static void Main()
//    {
//        // Disable console quick edit and mouse input
//        IntPtr consoleHandle = GetStdHandle(STD_INPUT_HANDLE);
//        uint consoleMode;
//        GetConsoleMode(consoleHandle, out consoleMode);
//        consoleMode &= ~(ENABLE_QUICK_EDIT | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT);
//        SetConsoleMode(consoleHandle, consoleMode);

//        Console.CursorVisible = false;
//        Console.Clear();
//        Console.WriteLine("XInput Direct Test - Press ESC to exit\n");

//        var state = new XInputState();

//        while (!Console.KeyAvailable || Console.ReadKey(true).Key != ConsoleKey.Escape)
//        {
//            int result = XInputGetState(0, ref state);

//            Console.SetCursorPosition(0, 3);

//            if (result == 0) // SUCCESS
//            {
//                var gp = state.Gamepad;

//                // Convert to float (-1 to 1)
//                float leftX = gp.sThumbLX / 32768f;
//                float leftY = gp.sThumbLY / 32768f;
//                float rightX = gp.sThumbRX / 32768f;
//                float rightY = gp.sThumbRY / 32768f;

//                Console.WriteLine($"Packet: {state.dwPacketNumber}");
//                Console.WriteLine($"Left:  X={leftX,7:F3} Y={leftY,7:F3}");
//                Console.WriteLine($"Right: X={rightX,7:F3} Y={rightY,7:F3}");
//                Console.WriteLine($"Triggers: L={gp.bLeftTrigger,3} R={gp.bRightTrigger,3}");
//                Console.WriteLine($"Buttons: 0x{gp.wButtons:X4}");

//                // Button states
//                Console.Write("Pressed: ");
//                if ((gp.wButtons & 0x1000) != 0) Console.Write("A ");
//                if ((gp.wButtons & 0x2000) != 0) Console.Write("B ");
//                if ((gp.wButtons & 0x4000) != 0) Console.Write("X ");
//                if ((gp.wButtons & 0x8000) != 0) Console.Write("Y ");
//                Console.WriteLine("          ");
//            }
//            else
//            {
//                Console.WriteLine("Controller disconnected     ");
//                Console.WriteLine("                           ");
//                Console.WriteLine("                           ");
//                Console.WriteLine("                           ");
//                Console.WriteLine("                           ");
//            }

//            Thread.Sleep(16);
//        }
//    }
//}


