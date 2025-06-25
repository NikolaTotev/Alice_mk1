using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System;
using System.IO.Ports;
using System.Threading;
using System.Threading.Tasks;
using System.Text;

public enum OperationalMode
{
    Idle = 0,
    DataAcquisition = 1,
    Control = 2,
    Debug = 3
}


public class RP2040SerialCommunicator
{
    private SerialPort _serialPort;
    private readonly object _lockObject = new object();
    private bool _isConnected = false;

    // Events for handling received data
    public event EventHandler<string> DataReceived;
    public event EventHandler<string> ErrorOccurred;
    public event EventHandler<OperationalMode> ModeChanged;

    public bool IsConnected => _isConnected && _serialPort?.IsOpen == true;
    public string PortName { get; private set; }

    public RP2040SerialCommunicator()
    {
        InitializeSerialPort();
    }

    private void InitializeSerialPort()
    {
        _serialPort = new SerialPort
        {
            BaudRate = 115200,  // Common baud rate for RP2040
            Parity = Parity.None,
            DataBits = 8,
            StopBits = StopBits.One,
            Handshake = Handshake.None,
            ReadTimeout = 1000,
            WriteTimeout = 1000,
            DtrEnable = true,
            RtsEnable = true,
        };

        //_serialPort.DsrHolding = false;
        //_serialPort.CtsHolding = false;

        _serialPort.DataReceived += OnDataReceived;
        _serialPort.ErrorReceived += OnErrorReceived;
    }

    public bool Connect(string portName)
    {
        try
        {
            lock (_lockObject)
            {

                _serialPort.PortName = portName;
                _serialPort.Open();

                PortName = portName;
                _isConnected = true;
                Console.WriteLine($"Serial port {_serialPort.IsOpen}");
                Console.WriteLine($"Connected to {portName}");
                return true;
            }
        }
        catch (Exception ex)
        {
            ErrorOccurred?.Invoke(this, $"Connection failed: {ex.Message}");
            return false;
        }
    }

    public void Disconnect()
    {
        try
        {
            lock (_lockObject)
            {
                if (_serialPort?.IsOpen == true)
                {
                    _serialPort.Close();
                }
                _isConnected = false;
                Console.WriteLine("Disconnected from serial port");
            }
        }
        catch (Exception ex)
        {
            ErrorOccurred?.Invoke(this, $"Disconnect error: {ex.Message}");
        }
    }

    // Command to change operational mode
    //public async Task<bool> ChangeOperationalMode(OperationalMode mode)
    //{
    //    if (!IsConnected) return false;

    //    try
    //    {
    //        string command = $"MODE:{(int)mode}\n";
    //        await SendCommandAsync(command);

    //        // Wait for confirmation (optional - depends on your Pico implementation)
    //        await Task.Delay(100);

    //        Console.WriteLine($"Mode change command sent: {mode}");
    //        return true;
    //    }
    //    catch (Exception ex)
    //    {
    //        ErrorOccurred?.Invoke(this, $"Mode change failed: {ex.Message}");
    //        return false;
    //    }
    //}

    // Send data to the Pico
    public async Task<bool> SendData(string data)
    {
        if (!IsConnected) return false;

        try
        {
            string command = $"DATA:{data}\n";
            await SendCommandAsync(command);
            Console.WriteLine($"Data sent: {data}");
            return true;
        }
        catch (Exception ex)
        {
            ErrorOccurred?.Invoke(this, $"Send data failed: {ex.Message}");
            return false;
        }
    }

    //// Send raw bytes
    //public async Task<bool> SendBytes(byte[] data)
    //{
    //    if (!IsConnected) return false;

    //    try
    //    {
    //        await Task.Run(() =>
    //        {
    //            lock (_lockObject)
    //            {
    //                _serialPort.Write(data, 0, data.Length);
    //            }
    //        });
    //        Console.WriteLine($"Bytes sent: {data.Length} bytes");
    //        return true;
    //    }
    //    catch (Exception ex)
    //    {
    //        ErrorOccurred?.Invoke(this, $"Send bytes failed: {ex.Message}");
    //        return false;
    //    }
    //}

    // Request data from the Pico
    public async Task<bool> RequestData(string dataType = "")
    {
        if (!IsConnected) return false;

        try
        {
            string command = string.IsNullOrEmpty(dataType) ? "GET\n" : $"GET:{dataType}\n";
            await SendCommandAsync(command);
            Console.WriteLine($"Data request sent: {dataType}");
            return true;
        }
        catch (Exception ex)
        {
            ErrorOccurred?.Invoke(this, $"Request data failed: {ex.Message}");
            return false;
        }
    }

    // Send a custom command
    public async Task<bool> SendCustomCommand(string command)
    {
        if (!IsConnected) return false;

        try
        {
            if (!command.EndsWith(">"))
                command += ">";

            await SendCommandAsync(command);
           // Console.WriteLine($"Custom command sent: {command.Trim()}");
            return true;
        }
        catch (Exception ex)
        {
            ErrorOccurred?.Invoke(this, $"Custom command failed: {ex.Message}");
            return false;
        }
    }

    // Read data asynchronously (with timeout)
    public async Task<string> ReadDataAsync(int timeoutMs = 10000)
    {
        if (!IsConnected) return null;

        try
        {
            return await Task.Run(() =>
            {
                try
                {
                    return _serialPort.ReadLine();
                }
                catch (Exception)
                {

                    return "";
                }

            });
        }
        catch (TimeoutException)
        {
            return null; // No data received within timeout
        }
        catch (Exception ex)
        {
            ErrorOccurred?.Invoke(this, $"Read data failed: {ex.Message}");
            return null;
        }
    }

    // Read all available data asynchronously
    public async Task<string> ReadAvailableDataAsync()
    {
        if (!IsConnected) return null;

        try
        {
            return await Task.Run(() => _serialPort.ReadExisting());
        }
        catch (Exception ex)
        {
            ErrorOccurred?.Invoke(this, $"Read available data failed: {ex.Message}");
            return null;
        }
    }

    private async Task SendCommandAsync(string command)
    {
        await Task.Run(() =>
        {
            lock (_lockObject)
            {
                if (_serialPort?.IsOpen == true)
                {
                    byte[] data = Encoding.UTF8.GetBytes(command);
                    Console.WriteLine($"Data length sent:{data.Length}");
                    try
                    {
                        _serialPort.Write(command);
                    }
                    catch (Exception)
                    {

                        Console.WriteLine("ERROR: Failed to write!");
                    }
                    
                }
            }
        });
    }

    private void OnDataReceived(object sender, SerialDataReceivedEventArgs e)
    {
        try
        {
            string data = _serialPort.ReadExisting();
            if (!string.IsNullOrEmpty(data))
            {
                ProcessReceivedData(data);
            }
        }
        catch (Exception ex)
        {
            ErrorOccurred?.Invoke(this, $"Data receive error: {ex.Message}");
        }
    }

    private void ProcessReceivedData(string data)
    {
        // Process different types of responses from the Pico
        string[] lines = data.Split(new[] { '\n', '\r' }, StringSplitOptions.RemoveEmptyEntries);

        foreach (string line in lines)
        {
            if (line.StartsWith("MODE_CHANGED:"))
            {
                if (int.TryParse(line.Substring(13), out int modeValue))
                {
                    OperationalMode newMode = (OperationalMode)modeValue;
                    ModeChanged?.Invoke(this, newMode);
                    Console.WriteLine($"Mode changed to: {newMode}");
                }
            }
            else if (line.StartsWith("ERROR:"))
            {
                ErrorOccurred?.Invoke(this, line.Substring(6));
            }
            else
            {
                // Regular data received
                DataReceived?.Invoke(this, line);
                //Console.WriteLine($"Data received: {line}");
            }
        }
    }

    private void OnErrorReceived(object sender, SerialErrorReceivedEventArgs e)
    {
        ErrorOccurred?.Invoke(this, $"Serial error: {e.EventType}");
    }

    public void Dispose()
    {
        Disconnect();
        _serialPort?.Dispose();
    }

    // Utility method to get available COM ports
    public static string[] GetAvailablePorts()
    {
        return SerialPort.GetPortNames();
    }
}

