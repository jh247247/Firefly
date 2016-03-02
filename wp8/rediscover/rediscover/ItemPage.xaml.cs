using rediscover.Common;
using rediscover.Data;
using rediscover.DataModel;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Windows.Input;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Graphics.Display;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Pivot Application template is documented at http://go.microsoft.com/fwlink/?LinkID=391641

namespace rediscover
{
    /// <summary>
    /// A page that displays details for a single item within a group.
    /// </summary>
    public sealed partial class ItemPage : Page
    {
        private readonly NavigationHelper navigationHelper;
        private readonly ObservableDictionary defaultViewModel = new ObservableDictionary();

        private Firefly fireflyClicked;

        public ItemPage()
        {
            this.InitializeComponent();

            this.navigationHelper = new NavigationHelper(this);
            this.navigationHelper.LoadState += this.NavigationHelper_LoadState;
            this.navigationHelper.SaveState += this.NavigationHelper_SaveState;
        } 
        
        public NavigationHelper NavigationHelper
        {
            get { return this.navigationHelper; }
        }
        
        public ObservableDictionary DefaultViewModel
        {
            get { return this.defaultViewModel; }
        }
        
        private async void NavigationHelper_LoadState(object sender, LoadStateEventArgs e)
        {
            fireflyClicked = (Firefly) e.NavigationParameter;

            // Load details
            tblId.Text = fireflyClicked.Id;

            if (fireflyClicked.Attribute != "")
            {
                txtAttribute.Text = fireflyClicked.Attribute;
                txtAttribute.FontStyle = Windows.UI.Text.FontStyle.Normal;
            }
            else
            {
                txtAttribute.Text = "Not set!";
                txtAttribute.FontStyle = Windows.UI.Text.FontStyle.Italic;
            }

            if (fireflyClicked.LastUpdateTime != "")
            {
                tblLastUpdate.Text = fireflyClicked.LastUpdateTime;
                tblLastUpdate.FontStyle = Windows.UI.Text.FontStyle.Normal;
            }
            else
            {
                tblLastUpdate.Text = "Unknown";
                tblLastUpdate.FontStyle = Windows.UI.Text.FontStyle.Italic;
            }

            if (fireflyClicked.NodeId != "")
            {
                tblLocation.Text = fireflyClicked.NodeId;
                tblLocation.FontStyle = Windows.UI.Text.FontStyle.Normal;
            }
            else
            {
                tblLocation.Text = "Unknown";
                tblLocation.FontStyle = Windows.UI.Text.FontStyle.Italic;
            }
        }
        
        private void NavigationHelper_SaveState(object sender, SaveStateEventArgs e)
        {
            
        }

        #region NavigationHelper registration

        /// <summary>
        /// The methods provided in this section are simply used to allow
        /// NavigationHelper to respond to the page's navigation methods.
        /// <para>
        /// Page specific logic should be placed in event handlers for the  
        /// <see cref="NavigationHelper.LoadState"/>
        /// and <see cref="NavigationHelper.SaveState"/>.
        /// The navigation parameter is available in the LoadState method 
        /// in addition to page state preserved during an earlier session.
        /// </para>
        /// </summary>
        /// <param name="e">Provides data for navigation methods and event
        /// handlers that cannot cancel the navigation request.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            this.navigationHelper.OnNavigatedTo(e);
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            this.navigationHelper.OnNavigatedFrom(e);
        }

        #endregion

        private void txtAttribute_KeyUp(object sender, KeyRoutedEventArgs e)
        {
            if (e.Key == Windows.System.VirtualKey.Enter)
            {
                // Hide keyboard
                InputPane.GetForCurrentView().TryHide();

                // Remove Textbox focus
                LoseFocus(sender);
            }
        }

        private void txtAttribute_LostFocus(object sender, RoutedEventArgs e)
        {
            // Save
            fireflyClicked.Attribute = txtAttribute.Text;

            // Post

        }

        private void LoseFocus(object sender)
        {
            var control = sender as Control;
            var isTabStop = control.IsTabStop;
            control.IsTabStop = false;
            control.IsEnabled = false;
            control.IsEnabled = true;
            control.IsTabStop = isTabStop;
        }
    }
}