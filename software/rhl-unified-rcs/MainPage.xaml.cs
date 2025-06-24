using Windows.Gaming.Input;

namespace rhl_unified_rcs
{
    public partial class MainPage : ContentPage
    {
        int count = 0;
        public MainPage()
        {
            InitializeComponent();

            // Initialization code (same as before)
            count = RawGameController.RawGameControllers.Count;
        }

        private void OnCounterClicked(object sender, EventArgs e)
        {

            count = RawGameController.RawGameControllers.Count;
            count = RawGameController.RawGameControllers.Count;
            CounterBtn.Text = $"Controller count {count}";

            if (count == 0)
            {
                CounterBtn.Text = $"No oogie :(";
            }
            else
            {
                var js = RawGameController.RawGameControllers[0];

                var axes = new double[js.AxisCount];
                var buttons = new bool[js.ButtonCount];
                js.GetCurrentReading(buttons, null, axes);

                CounterBtn.Text = $"Inputs {axes[0]}";
            }




            SemanticScreenReader.Announce(CounterBtn.Text);
        }


        private static async Task InteractiveLoop(RawGameController js)
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
                        if (!isStopped)
                        {
                            //await communicator.SendCustomCommand("stop");
                            Console.WriteLine("Stopping");
                            isStopped = true;
                        }
                    }

                    if (axes[0] >= 0.6)
                    {
                        if (isStopped)
                        {
                            //await communicator.SendCustomCommand($"mcw_{Convert.ToInt32(mapped)}");
                            Console.WriteLine($"mcw_{Convert.ToInt32(mapped)}");
                            isStopped = false;
                        }
                        else
                        {
                            //await communicator.SendCustomCommand($"update-speed_{Convert.ToInt32(mapped)}");
                            Console.WriteLine($"mcw_{Convert.ToInt32(mapped)}");
                        }

                    }

                    if (axes[0] <= 0.4)
                    {
                        if (isStopped)
                        {
                            //await communicator.SendCustomCommand($"mccw_{Convert.ToInt32(mapped)}");
                            Console.WriteLine($"mccw_{Convert.ToInt32(mapped)}");
                            isStopped = false;
                        }

                        else
                        {
                            //await communicator.SendCustomCommand($"update-speed_{Convert.ToInt32(mapped)}");
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
                        //await communicator.SendCustomCommand("stop");
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
    }




}
