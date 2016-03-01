using rediscover.Common;
using rediscover.Data;
using rediscover.DataModel;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using Windows.ApplicationModel.Resources;
using Windows.Data.Json;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Graphics.Display;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.Web.Http;
using Zeroconf;

// The Pivot Application template is documented at http://go.microsoft.com/fwlink/?LinkID=391641

namespace rediscover
{
    public sealed partial class PivotPage : Page
    {
        private readonly NavigationHelper navigationHelper;
        private readonly ObservableDictionary defaultViewModel = new ObservableDictionary();

        public static FireflyCollection fireflies { get; set; }
        Windows.Storage.ApplicationDataContainer localSettings;
        SynchronizationContext _syncContext;

        public string monitorIPAddress = "192.168.2.81";
        public string monitorPort = "5000";
        public string monitorUri = "";

        public PivotPage()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = NavigationCacheMode.Required;

            this.navigationHelper = new NavigationHelper(this);
            this.navigationHelper.LoadState += this.NavigationHelper_LoadState;
            this.navigationHelper.SaveState += this.NavigationHelper_SaveState;

            _syncContext = SynchronizationContext.Current;

            fireflies = new FireflyCollection();
            addSampleFireflies();
            lstFireflies.ItemsSource = fireflies;

            monitorUri = "http://" + monitorIPAddress + ":" + monitorPort + "/";

            getMonitorIPAddress();

            //getFirefliesFromMonitor();
        }

        public async void getMonitorIPAddress()
        {
            var domains = await ZeroconfResolver.BrowseDomainsAsync();

            //var responses = await ZeroconfResolver.ResolveAsync(domains.Select(g => g.Key));
            var responses = await ZeroconfResolver.ResolveAsync("_rediscover._tcp.local."); // "_http._tcp"

            foreach (var response in responses)
            {
                fireflies.Add(new Firefly("Zeroconf Responses", response.IPAddress));
            }
            RefreshFirefliesListView();

            foreach (var domain in domains)
            {
                fireflies.Add(new Firefly("Zeroconf Domains", domain.ToString()));
            }
            RefreshFirefliesListView();
        }

        public async void getFirefliesFromMonitor()
        {
            // Http Get Request
            var client = new HttpClient();
            Uri uri = new Uri(monitorUri + "firefly");
            var response = await client.GetAsync(uri);
            
            if (response.IsSuccessStatusCode) // Get Success
            {
                // Get Json
                string content = await response.Content.ReadAsStringAsync();

                // Parse Json
                JsonObject jsonParsed = await Task.Run(() => JsonObject.Parse(content));
                JsonArray fireflyIds = jsonParsed.GetNamedArray("fireflyIds");

                // Create fireflies from ids
                foreach (JsonValue firefly in fireflyIds)
                {
                    fireflies.Add(new Firefly(firefly.GetString()));
                }
                RefreshFirefliesListView();
            }
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
            
        }
        
        private void NavigationHelper_SaveState(object sender, SaveStateEventArgs e)
        {
            
        }

        private void btnSettings_Click(object sender, RoutedEventArgs e)
        {
            //Frame.Navigate(typeof(SettingsPage));
        }

        private void btnSearch_Click(object sender, RoutedEventArgs e)
        {
            //Frame.Navigate(typeof(SearchPage));
        }

        private void pvtMain_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (((Pivot)sender).SelectedIndex == 0)
            {
                // Refresh Fireflies
            }
            else if (((Pivot)sender).SelectedIndex == 1)
            {
                // Refresh Nodes
            }
        }

        #region Firefly ListView

        private void lstFireflies_ItemClick(object sender, ItemClickEventArgs e)
        {
            var fireflyClicked = ((Firefly) e.ClickedItem);
            if (!Frame.Navigate(typeof(ItemPage), fireflyClicked))
            {
                throw new Exception("Navigation Failed Exception");
            }
        }

        // Load listview contents in stages for user-responsiveness
        private void lstFireflies_ContainerContentChanging(ListViewBase sender, ContainerContentChangingEventArgs args)
        {
            ItemViewerFirefly iv = args.ItemContainer.ContentTemplateRoot as ItemViewerFirefly;

            if (args.InRecycleQueue == true)
            {
                iv.ClearData();
            }
            else if (args.Phase == 0)
            {
                iv.ShowPlaceholder(args.Item as Firefly);
                args.RegisterUpdateCallback(ContainerContentChangingDelegate);
            }
            else if (args.Phase == 1)
            {
                iv.ShowId();
                args.RegisterUpdateCallback(ContainerContentChangingDelegate);
            }
            else if (args.Phase == 2)
            {
                iv.ShowAttribute();
            }

            args.Handled = true;
        }
        private TypedEventHandler<ListViewBase, ContainerContentChangingEventArgs> ContainerContentChangingDelegate
        {
            get
            {
                if (_delegate == null)
                {
                    _delegate = new TypedEventHandler<ListViewBase, ContainerContentChangingEventArgs>(lstFireflies_ContainerContentChanging);
                }
                return _delegate;
            }
        }
        private TypedEventHandler<ListViewBase, ContainerContentChangingEventArgs> _delegate;

        private void RefreshFirefliesListView()
        {
            _syncContext.Post((s) =>
            {
                // TODO: Find better way of refreshing listview
                lstFireflies.ItemsSource = null;
                lstFireflies.ItemsSource = fireflies;
            }, null);
        }

        #endregion

        private void addSampleFireflies()
        {
            Firefly firefly = new Firefly("123", "Thermometer 1");
            fireflies.Add(firefly);
            firefly = new Firefly("456", "Air bed 2");
            fireflies.Add(firefly);
            firefly = new Firefly("789", "Clipboard 3");
            fireflies.Add(firefly);
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
    }
}
