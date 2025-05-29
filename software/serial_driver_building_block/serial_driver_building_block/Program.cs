internal class Program
{
    // Method 1: Async Main (C# 7.1+) - RECOMMENDED
    private static async Task Main(string[] args)
    {
        Console.WriteLine("=== RP2040 Serial Communication Example ===\n");
        
        var communicator = new RP2040SerialCommunicator();             

        // Show available ports
        Console.WriteLine("Available COM ports:");
        foreach (string port in RP2040SerialCommunicator.GetAvailablePorts())
        {
            Console.WriteLine($"  {port}");
        }

        Console.WriteLine("Select port to use...");
        string comPort = Console.ReadLine();

        // Connect to the Pico (adjust COM port as needed)
        //string comPort = "COM3"; // Change this to your Pico's COM port
        Console.WriteLine($"\nAttempting to connect to {comPort}...");
        
        if (communicator.Connect(comPort))
        {
            Console.WriteLine("Connected successfully!\n");
            
            // Example operations with proper async/await
            await Task.Delay(1000); // Give time for connection to stabilize
            Console.WriteLine("Sending start command");
            await communicator.SendCustomCommand("start_cli>");
            await Task.Delay(1000); // Give time for connection to stabilize


            // Try to read response asynchronously
            Console.WriteLine("\nAttempting to read response...");
            string response = await communicator.ReadDataAsync(2000);
            if (response != null)
            {
                Console.WriteLine($"Synchronous read: {response}");
            }
            else
            {
                Console.WriteLine("No response received within timeout");
            }

            

            // Keep the program running to receive data
            Console.WriteLine("\n=== Communication established. Monitoring for data... ===");
            Console.WriteLine("Press any key to exit...\n");
            Console.ReadKey();

            communicator.Disconnect();
        }
        else
        {
            Console.WriteLine($"Failed to connect to {comPort}");
        }

        communicator.Dispose();
        Console.WriteLine("Application closed.");
    }
}