using rediscover.Common;
using rediscover.Data;
using rediscover.DataModel;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
//using System.Net.Http;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using Windows.ApplicationModel.Resources;
using Windows.Data.Json;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Graphics.Display;
using Windows.Storage.Streams;
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

        public FireflyCollection fireflies { get; set; }
        public NodeCollection nodes { get; set; }
        Windows.Storage.ApplicationDataContainer localSettings;
        SynchronizationContext _syncContext;

        public string monitorIPAddress = "192.168.2.81";
        public string monitorPort = "5000";
        public string monitorUri = "";

        public static string CURL_MEDIA_TYPE = "application/x-www-form-urlencoded"; // Only one that works!!!

        public PivotPage()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = NavigationCacheMode.Required;

            this.navigationHelper = new NavigationHelper(this);
            this.navigationHelper.LoadState += this.NavigationHelper_LoadState;
            this.navigationHelper.SaveState += this.NavigationHelper_SaveState;

            _syncContext = SynchronizationContext.Current;

            fireflies = new FireflyCollection();
            //addSampleFireflies();
            lstFireflies.ItemsSource = fireflies;

            nodes = new NodeCollection();
            lstNodes.ItemsSource = nodes;

            monitorUri = "http://" + monitorIPAddress + ":" + monitorPort + "/";

            //getMonitorIPAddress();

            getFirefliesFromMonitor();
            getNodesFromMonitor();
        }

        #region GET and PUT functions

        // TODO: Get working with rediscover
        public async void getMonitorIPAddress()
        {
            var domains = await ZeroconfResolver.BrowseDomainsAsync();

            //var responses = await ZeroconfResolver.ResolveAsync(domains.Select(g => g.Key));
            var responses = await ZeroconfResolver.ResolveAsync("_http._tcp.local."); // This finds Brother printer
            //var responses = await ZeroconfResolver.ResolveAsync("_rediscover._tcp.local."); // "_http._tcp"

            foreach (var response in responses)
            {
                fireflies.Add(new Firefly("Zeroconf Responses", response.DisplayName));
            }
            RefreshFirefliesListView();

            foreach (var domain in domains.Select(g => g.Key))
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
            Debug.WriteLine("Http GET of fireflies from monitor: " + uri);
            var response = await client.GetAsync(uri); // TODO: If System.Exception occurs here, bc monitor not running, check before get here
            
            if (response.IsSuccessStatusCode) // Get Success
            {
                // Get Json
                string content = await response.Content.ReadAsStringAsync();

                // Parse Json
                JsonObject jsonParsed = await Task.Run(() => JsonObject.Parse(content));
                JsonArray fireflyIds = jsonParsed.GetNamedArray("ids");

                // Create fireflies from ids
                foreach (JsonValue fireflyId in fireflyIds)
                {
                    getFireflyDetailsFromMonitor(fireflyId.GetString());
                }
                RefreshFirefliesListView();
            }
            else
            {
                Debug.WriteLine("Error: Http GET of fireflies from monitor failed: " + uri);
            }
        }

        public async void getFireflyDetailsFromMonitor(string fireflyId)
        {
            // Http Get Request
            var client = new HttpClient();
            Uri uri = new Uri(monitorUri + "firefly/" + fireflyId);
            Debug.WriteLine("Http GET of firefly details from monitor: " + uri);
            var response = await client.GetAsync(uri);

            if (response.IsSuccessStatusCode) // Get Success
            {
                // Get Json
                string content = await response.Content.ReadAsStringAsync();

                // Parse Json
                JsonObject jsonParsed = await Task.Run(() => JsonObject.Parse(content));
                Debug.WriteLine(jsonParsed.GetNamedString("fireflyId"));

                Firefly newFirefly = new Firefly();
                try
                {
                    newFirefly.Id = jsonParsed.GetNamedString("fireflyId");
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Warning: No id associated with firefly");
                }
                try
                {
                    newFirefly.Battery = jsonParsed.GetNamedString("bat");
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Warning: No battery value associated with firefly");
                }
                try
                {
                    var timestamp = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
                    timestamp = timestamp.AddSeconds(jsonParsed.GetNamedNumber("timestamp"));
                    newFirefly.LastUpdateTime = timestamp.ToLocalTime();
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Warning: No timestamp associated with firefly");
                }
                try
                {
                    newFirefly.NodeId = jsonParsed.GetNamedString("nodeId");
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Warning: No nodeId associated with firefly");
                }
                try
                {
                    JsonObject usrData = jsonParsed["usrData"].GetObject();
                    newFirefly.Attribute = usrData.GetNamedString("attribute");
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Warning: No attribute associated with firefly");
                }

                fireflies.Add(newFirefly);
                RefreshFirefliesListView();
            }
            else
            {
                Debug.WriteLine("Error: Http GET of firefly details from monitor failed: " + uri);
            }
        }

        public async void getNodesFromMonitor()
        {
            // Http Get Request
            var client = new HttpClient();
            Uri uri = new Uri(monitorUri + "node");
            Debug.WriteLine("Http GET of nodes from monitor: " + uri);
            var response = await client.GetAsync(uri);

            if (response.IsSuccessStatusCode) // Get Success
            {
                // Get Json
                string content = await response.Content.ReadAsStringAsync();

                // Parse Json
                JsonObject jsonParsed = await Task.Run(() => JsonObject.Parse(content));
                JsonArray nodeIds = jsonParsed.GetNamedArray("ids");

                // Create fireflies from ids
                foreach (JsonValue nodeId in nodeIds)
                {
                    getNodeDetailsFromMonitor(nodeId.GetString());
                }
                RefreshNodesListView();
            }
            else
            {
                Debug.WriteLine("Error: Http GET of nodes from monitor failed: " + uri);
            }
        }

        public async void getNodeDetailsFromMonitor(string nodeId)
        {
            // Http Get Request
            var client = new HttpClient();
            Uri uri = new Uri(monitorUri + "node/" + nodeId);
            Debug.WriteLine("Http GET of node details from monitor: " + uri);
            var response = await client.GetAsync(uri);

            if (response.IsSuccessStatusCode) // Get Success
            {
                // Get Json
                string content = await response.Content.ReadAsStringAsync();

                // Parse Json
                JsonObject jsonParsed = await Task.Run(() => JsonObject.Parse(content));
                Debug.WriteLine(jsonParsed.GetNamedString("nodeId"));

                Node newNode = new Node();
                try
                {
                    newNode.Id = jsonParsed.GetNamedString("nodeId");
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Warning: No id associated with node");
                }
                try
                {
                    var timestamp = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
                    timestamp = timestamp.AddSeconds(jsonParsed.GetNamedNumber("timestamp"));
                    newNode.LastUpdateTime = timestamp.ToLocalTime();
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Warning: No timestamp associated with node");
                }
                try
                {
                    newNode.FirefliesList = jsonParsed.GetNamedArray("fireflies");
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Warning: No fireflies list associated with node");
                }
                try
                {
                    JsonObject usrData = jsonParsed["usrData"].GetObject();
                    newNode.Location = usrData.GetNamedString("location");
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Warning: No location associated with node");
                }

                nodes.Add(newNode);
                RefreshNodesListView();
            }
            else
            {
                Debug.WriteLine("Error: Http GET of node details from monitor failed: " + uri);
            }
        }

        public async void putFireflyAttribute(Firefly fireflyToUpate)
        {
            // PUT data
            Uri uri = new Uri(monitorUri + "firefly/" + fireflyToUpate.Id);
            string usrData = "attribute=" + fireflyToUpate.Attribute;

            // Http PUT Request
            HttpClient client = new HttpClient();
            HttpStringContent content = new HttpStringContent(usrData, UnicodeEncoding.Utf8, CURL_MEDIA_TYPE);
            HttpResponseMessage response = await client.PutAsync(uri, content);

            if (response.IsSuccessStatusCode) // Success
            {
                Debug.WriteLine("Put Success: " + fireflyToUpate.Id);
            }
            else
            {
                Debug.WriteLine("Put Fail: " + fireflyToUpate.Id);
            }
        }

        public async void putNodeLocation(Node nodeToUpate)
        {
            // PUT data
            Uri uri = new Uri(monitorUri + "node/" + nodeToUpate.Id);
            string usrData = "location=" + nodeToUpate.Location;

            // Http PUT Request
            HttpClient client = new HttpClient();
            HttpStringContent content = new HttpStringContent(usrData, UnicodeEncoding.Utf8, CURL_MEDIA_TYPE);
            HttpResponseMessage response = await client.PutAsync(uri, content);

            if (response.IsSuccessStatusCode) // Success
            {
                Debug.WriteLine("Put Success: " + nodeToUpate.Id);
            }
            else
            {
                Debug.WriteLine("Put Fail: " + nodeToUpate.Id);
            }
        }

        #endregion

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
            // Get firefly
            var fireflyClicked = ((Firefly)e.ClickedItem);

            // Setup Navigation args
            List<object> itemPageNavList = new List<object>();
            itemPageNavList.Add(this);
            itemPageNavList.Add(fireflyClicked);
            
            // Navigate
            if (!Frame.Navigate(typeof(ItemPageFirefly), itemPageNavList))
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
                args.RegisterUpdateCallback(ContainerContentChangingDelegateFireflies);
            }
            else if (args.Phase == 1)
            {
                iv.ShowId();
                args.RegisterUpdateCallback(ContainerContentChangingDelegateFireflies);
            }
            else if (args.Phase == 2)
            {
                iv.ShowAttribute();
            }

            args.Handled = true;
        }
        private TypedEventHandler<ListViewBase, ContainerContentChangingEventArgs> ContainerContentChangingDelegateFireflies
        {
            get
            {
                if (_delegateFireflies == null)
                {
                    _delegateFireflies = new TypedEventHandler<ListViewBase, ContainerContentChangingEventArgs>(lstFireflies_ContainerContentChanging);
                }
                return _delegateFireflies;
            }
        }
        private TypedEventHandler<ListViewBase, ContainerContentChangingEventArgs> _delegateFireflies;

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

        #region Node ListView

        private void lstNodes_ItemClick(object sender, ItemClickEventArgs e)
        {
            // Get node
            var nodeClicked = ((Node)e.ClickedItem);

            // Setup Navigation args
            List<object> itemPageNavList = new List<object>();
            itemPageNavList.Add(this);
            itemPageNavList.Add(nodeClicked);

            // Navigate
            if (!Frame.Navigate(typeof(ItemPageNode), itemPageNavList))
            {
                throw new Exception("Navigation Failed Exception");
            }
        }

        // Load listview contents in stages for user-responsiveness
        private void lstNodes_ContainerContentChanging(ListViewBase sender, ContainerContentChangingEventArgs args)
        {
            ItemViewerNode iv = args.ItemContainer.ContentTemplateRoot as ItemViewerNode;

            if (args.InRecycleQueue == true)
            {
                iv.ClearData();
            }
            else if (args.Phase == 0)
            {
                iv.ShowPlaceholder(args.Item as Node);
                args.RegisterUpdateCallback(ContainerContentChangingDelegateNodes);
            }
            else if (args.Phase == 1)
            {
                iv.ShowId();
                args.RegisterUpdateCallback(ContainerContentChangingDelegateNodes);
            }
            else if (args.Phase == 2)
            {
                iv.ShowLocation();
            }

            args.Handled = true;
        }
        private TypedEventHandler<ListViewBase, ContainerContentChangingEventArgs> ContainerContentChangingDelegateNodes
        {
            get
            {
                if (_delegateNodes == null)
                {
                    _delegateNodes = new TypedEventHandler<ListViewBase, ContainerContentChangingEventArgs>(lstNodes_ContainerContentChanging);
                }
                return _delegateNodes;
            }
        }
        private TypedEventHandler<ListViewBase, ContainerContentChangingEventArgs> _delegateNodes;

        private void RefreshNodesListView()
        {
            _syncContext.Post((s) =>
            {
                // TODO: Find better way of refreshing listview
                lstNodes.ItemsSource = null;
                lstNodes.ItemsSource = nodes;
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

        #region NavigationHelper and more

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
            RefreshFirefliesListView();
            RefreshNodesListView();
        }

        private void NavigationHelper_SaveState(object sender, SaveStateEventArgs e)
        {

        }

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
