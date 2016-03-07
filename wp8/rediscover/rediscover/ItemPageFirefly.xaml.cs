using rediscover.Common;
using rediscover.DataModel;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using System.Windows.Input;
using Windows.Data.Json;
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
using Windows.Web.Http;

// The Pivot Application template is documented at http://go.microsoft.com/fwlink/?LinkID=391641

namespace rediscover
{
    /// <summary>
    /// A page that displays details for a single item within a group.
    /// </summary>
    public sealed partial class ItemPageFirefly : Page
    {
        private readonly NavigationHelper navigationHelper;
        private readonly ObservableDictionary defaultViewModel = new ObservableDictionary();

        private Firefly fireflyClicked;
        private PivotPage pivotPageRef;

        private DispatcherTimer dispatcherTimer;
        private HttpClient client;

        public ItemPageFirefly()
        {
            this.InitializeComponent();

            this.navigationHelper = new NavigationHelper(this);
            this.navigationHelper.LoadState += this.NavigationHelper_LoadState;
            this.navigationHelper.SaveState += this.NavigationHelper_SaveState;

            var httpFilter = new Windows.Web.Http.Filters.HttpBaseProtocolFilter();
            httpFilter.CacheControl.ReadBehavior =
                Windows.Web.Http.Filters.HttpCacheReadBehavior.MostRecent;

            client = new HttpClient(httpFilter);

            dispatcherTimer = new DispatcherTimer();
            dispatcherTimer.Tick += dispatcherTimer_Tick;
            dispatcherTimer.Interval = new TimeSpan(0, 0, 5);
            dispatcherTimer.Start();
        } 
        
        public NavigationHelper NavigationHelper
        {
            get { return this.navigationHelper; }
        }
        
        public ObservableDictionary DefaultViewModel
        {
            get { return this.defaultViewModel; }
        }
        
        private void NavigationHelper_LoadState(object sender, LoadStateEventArgs e)
        {
            // Parse Navigation Args
            List<object> itemPageNavList = (List<object>) e.NavigationParameter;

            pivotPageRef = (PivotPage) itemPageNavList.ElementAt<object>(0);
            fireflyClicked = (Firefly) itemPageNavList.ElementAt<object>(1);

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

            if (fireflyClicked.LastUpdateTime != new DateTime(0))
            {
                tblLastUpdate.Text = fireflyClicked.LastUpdateTime.ToString();
                tblLastUpdate.FontStyle = Windows.UI.Text.FontStyle.Normal;
            }
            else
            {
                tblLastUpdate.Text = "Unknown";
                tblLastUpdate.FontStyle = Windows.UI.Text.FontStyle.Italic;
            }

            setLocationText();
        }
        
        private void NavigationHelper_SaveState(object sender, SaveStateEventArgs e)
        {
            
        }

        #region NavigationHelper registration
        
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

            // Put
            pivotPageRef.putFireflyAttribute(fireflyClicked);
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

        private void dispatcherTimer_Tick(object sender, object e)
        {
            getFireflyLocationFromMonitor();
        }

        public async void getFireflyLocationFromMonitor()
        {
            // Http Get Request
            Uri uri = new Uri(pivotPageRef.monitorUri + "firefly/" + fireflyClicked.Id);
            Debug.WriteLine("Info: Http GET of firefly location from monitor: " + uri);
            var response = await client.GetAsync(uri);

            if (response.IsSuccessStatusCode) // Get Success
            {
                // Get Json
                string content = await response.Content.ReadAsStringAsync();

                // Parse Json
                JsonObject jsonParsed = await Task.Run(() => JsonObject.Parse(content));

                string nodeIdFromGet = "";
                try
                {
                    nodeIdFromGet = jsonParsed.GetNamedString("nodeId");
                }
                catch
                {
                }

                if (nodeIdFromGet != fireflyClicked.NodeId)
                {
                    fireflyClicked.NodeId = nodeIdFromGet;
                    setLocationText();
                }
            }
            else
            {
                Debug.WriteLine("Error: Http GET of firefly location from monitor failed: " + uri);
            }
        }

        private void setLocationText()
        {
            if (fireflyClicked.NodeId != "")
            {
                string location = pivotPageRef.nodes.GetLocationOfNode(fireflyClicked.NodeId);
                if (location != "")
                {
                    tblLocation.Text = location;
                    tblLocation.FontStyle = Windows.UI.Text.FontStyle.Normal;
                }
                else
                {
                    tblLocation.Text = "At Node " + fireflyClicked.NodeId;
                    tblLocation.FontStyle = Windows.UI.Text.FontStyle.Italic;
                }
            }
            else
            {
                tblLocation.Text = "Unknown";
                tblLocation.FontStyle = Windows.UI.Text.FontStyle.Italic;
            }
        }
    }
}