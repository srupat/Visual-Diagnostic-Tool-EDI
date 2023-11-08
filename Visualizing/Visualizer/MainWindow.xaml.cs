using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Runtime.InteropServices;
using System.Diagnostics.CodeAnalysis;
using System.Diagnostics;
using System.Windows.Media.Animation;
using System.Reflection.PortableExecutable;
using System.Reflection.Metadata;

namespace Visualizer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        /* [DllImport("ExportStackTrace.dll", CallingConvention = CallingConvention.StdCall)]
         static extern IntPtr getProcessHandle(); // Assuming the function returns a process handle as an IntPtr.

         [DllImport("ExportStackTrace.dll", CallingConvention = CallingConvention.StdCall)]
         static extern void Mult(int num1, int num2);
        */

        [return: MarshalAs(UnmanagedType.BStr)]
        [DllImport("ExportStackTrace.dll", CallingConvention = CallingConvention.StdCall)]
        static extern string GetFunctionName(int str_count);

        [DllImport("ExportStackTrace.dll", CallingConvention = CallingConvention.StdCall)]
        static extern int GetFunctionCount();

        public MainWindow()
        {
            InitializeComponent();

            // Calculate the desired proportion of the screen size
            double widthProportion = 0.7; // 70% of the screen width
            double heightProportion = 0.6; // 60% of the screen height

            // Get the screen dimensions
            double screenWidth = SystemParameters.PrimaryScreenWidth;
            double screenHeight = SystemParameters.PrimaryScreenHeight;

            // Calculate the window width and height based on the proportions
            double windowWidth = screenWidth * widthProportion;
            double windowHeight = screenHeight * heightProportion;

            // Set the window dimensions
            Width = windowWidth;
            Height = windowHeight;

            Title = "My Call Stack"; // Set the window title
            WindowStartupLocation = WindowStartupLocation.CenterScreen; // Set the window startup location
            WindowState = WindowState.Normal; // Set the initial window state
            WindowStyle = WindowStyle.SingleBorderWindow; // Set the window style
            ResizeMode = ResizeMode.NoResize; // Set the resize mode
            Background = System.Windows.Media.Brushes.LightGray; // Set the background color

            // Create a Grid to host the button and stack
            Grid grid = new Grid();
            Content = grid;

            // Create a button
            Button myButton = new Button
            {
                Content = "View Stack", // Set the button text
                Width = 100,           // Set the button width
                Height = 30,            // Set the button height
                HorizontalAlignment = HorizontalAlignment.Right,
                VerticalAlignment = VerticalAlignment.Top,   
            };

            // Add a click event handler to the button
            myButton.Click += MyButton_Click;

            // Add the button to the Grid
            grid.Children.Add(myButton);
        }

        private void MyButton_Click(object sender, RoutedEventArgs e)
        {
            int expanderCount = GetFunctionCount();
            Grid grid = Content as Grid;
            if (stackPanel != null)
            {
                grid.Children.Remove(stackPanel);
            }

            stackPanel = new StackPanel
            {
                Orientation = Orientation.Vertical,
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Center
            };

            int defaultExpanderWidth = 300; // Set a default width for all expanders
            int defaultExpanderHeight = 60; // Set a default height for all expanders
            int expandedExpanderWidth = 400; // Set an expanded width
            int expandedExpanderHeight = 100; // Set an expanded height
            TimeSpan duration = TimeSpan.FromSeconds(0.3); // Duration for the transition animation

            for (int i = 0; i < expanderCount; i++)
            {
                Expander expander = new Expander
                {
                    IsExpanded = false,
                    Padding = new Thickness(10),
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Center,
                    Background = i == 0 ? new SolidColorBrush(Colors.LightGreen) : new SolidColorBrush(Colors.DarkGray),
                    BorderBrush = new SolidColorBrush(Colors.Black),
                    Margin = new Thickness(10, 5, 10, 5),
                    Height = defaultExpanderHeight
                };

                TextBlock textBlock = new TextBlock
                {
                    Text = $"{i}",
                    Padding = new Thickness(10)
                };
                expander.Content = textBlock;

                TextBlock headerTextBlock = new TextBlock
                {
                    Text = GetFunctionName(i),
                    FontWeight = FontWeights.Bold,
                    FontSize = 16
                };
                expander.Header = headerTextBlock;

                // Smooth transition animation for expanding
                DoubleAnimation expandHeightAnimation = new DoubleAnimation(defaultExpanderHeight, expandedExpanderHeight, duration);

                // Smooth transition animation for collapsing
                DoubleAnimation collapseHeightAnimation = new DoubleAnimation(expandedExpanderHeight, defaultExpanderHeight, duration);

                expander.Expanded += (s, ev) =>
                {
                    expander.BeginAnimation(Expander.HeightProperty, expandHeightAnimation);
                };

                expander.Collapsed += (s, ev) =>
                {
                    expander.BeginAnimation(Expander.HeightProperty, collapseHeightAnimation);
                };

                stackPanel.Children.Add(expander);
            }

            // Get the Grid that contains the button and add the stack to it
            grid.Children.Add(stackPanel);
        }

    }

}
