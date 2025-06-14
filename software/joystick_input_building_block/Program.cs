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

        Console.Clear();
        Console.WriteLine($"Controller: {js.DisplayName ?? "Unknown"}\n");

        while (!Console.KeyAvailable)
        {
            js.GetCurrentReading(buttons, null, axes);
            Console.SetCursorPosition(0, 2);

            // Display mapped axes
            Console.WriteLine("MAPPED AXES:");
            for (int i = 0; i < axes.Length; i++)
            {
                double mapped = MapAxisValue(axes[i]);
                Console.WriteLine($"  Axis {i}: {mapped,4:F0} (raw: {axes[i]:F3})");
            }

            // Buttons
            Console.Write("\nButtons: ");
            for (int i = 0; i < buttons.Length; i++)
            {
                if (buttons[i]) Console.Write($"[{i}] ");
            }
            Console.WriteLine("        ");

            Thread.Sleep(20);
        }
    }

    static double MapAxisValue(double rawValue)
    {
        return rawValue < 0.6 ? 0 : 250 + ((rawValue - 0.6) / 0.4 * 350);
    }

    
}