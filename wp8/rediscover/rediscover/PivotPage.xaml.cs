using rediscover.Common;
using rediscover.DataModel;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
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
            
            getMonitorIPAddress();

            monitorUri = "http://" + monitorIPAddress + ":" + monitorPort + "/";

            getFirefliesFromMonitor();
            getNodesFromMonitor();
        }

        #region GET and PUT functions
        
        public async void getMonitorIPAddress()
        {
            var responses = await ZeroconfResolver.ResolveAsync("_rediscover._tcp.local");

            foreach (var response in responses)
            {
                // Debug output
                Debug.WriteLine("Info: Zeroconf Response: " + response.DisplayName + ": " + response.IPAddress);

                // Update IP address
                monitorIPAddress = response.IPAddress;
            }
        }

        public async void getFirefliesFromMonitor()
        {
            // Http Get Request
            var client = new HttpClient();
            Uri uri = new Uri(monitorUri + "firefly");
            Debug.WriteLine("Info: Http GET of fireflies from monitor: " + uri);

            try
            {
                var response = await client.GetAsync(uri);
            
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
                    Debug.WriteLine("Error: During Firefly GET. Error code: " + response.StatusCode + ": " + uri);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Error: During Firefly GET. Possibly unable to connect to monitor: " + ex.Message);
            }
        }

        public async void getFireflyDetailsFromMonitor(string fireflyId)
        {
            // Http Get Request
            var client = new HttpClient();
            Uri uri = new Uri(monitorUri + "firefly/" + fireflyId);
            Debug.WriteLine("Info: Http GET of firefly details from monitor: " + uri);
            var response = await client.GetAsync(uri);

            if (response.IsSuccessStatusCode) // Get Success
            {
                // Get Json
                string content = await response.Content.ReadAsStringAsync();

                // Parse Json
                JsonObject jsonParsed = await Task.Run(() => JsonObject.Parse(content));

                Firefly newFirefly = new Firefly();
                try
                {
                    newFirefly.Id = jsonParsed.GetNamedString("fireflyId");
                }
                catch
                {
                    Debug.WriteLine("Warning: No id associated with firefly");
                }
                try
                {
                    newFirefly.Battery = jsonParsed.GetNamedString("bat");
                }
                catch
                {
                }
                try
                {
                    var timestamp = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
                    timestamp = timestamp.AddSeconds(jsonParsed.GetNamedNumber("timestamp"));
                    newFirefly.LastUpdateTime = timestamp.ToLocalTime();
                }
                catch
                {
                }
                try
                {
                    newFirefly.NodeId = jsonParsed.GetNamedString("nodeId");
                }
                catch
                {
                }
                try
                {
                    JsonObject usrData = jsonParsed["usrData"].GetObject();
                    newFirefly.Attribute = usrData.GetNamedString("attribute");
                }
                catch
                {
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
            Debug.WriteLine("Info: Http GET of nodes from monitor: " + uri);
            try
            {
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
                    Debug.WriteLine("Error: During Node GET. Error code: " + response.StatusCode + ": " + uri);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Error: During Node GET. Possibly unable to connect to monitor: " + ex.Message);
            }
        }

        public async void getNodeDetailsFromMonitor(string nodeId)
        {
            // Http Get Request
            var client = new HttpClient();
            Uri uri = new Uri(monitorUri + "node/" + nodeId);
            Debug.WriteLine("Info: Http GET of node details from monitor: " + uri);
            var response = await client.GetAsync(uri);

            if (response.IsSuccessStatusCode) // Get Success
            {
                // Get Json
                string content = await response.Content.ReadAsStringAsync();

                // Parse Json
                JsonObject jsonParsed = await Task.Run(() => JsonObject.Parse(content));

                Node newNode = new Node();
                try
                {
                    newNode.Id = jsonParsed.GetNamedString("nodeId");
                }
                catch
                {
                    Debug.WriteLine("Warning: No id associated with node");
                }
                try
                {
                    var timestamp = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
                    timestamp = timestamp.AddSeconds(jsonParsed.GetNamedNumber("timestamp"));
                    newNode.LastUpdateTime = timestamp.ToLocalTime();
                }
                catch
                {
                }
                try
                {
                    newNode.FirefliesList = jsonParsed.GetNamedArray("fireflies");
                }
                catch
                {
                }
                try
                {
                    JsonObject usrData = jsonParsed["usrData"].GetObject();
                    newNode.Location = usrData.GetNamedString("location");
                }
                catch
                {
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
                Debug.WriteLine("Info: Put Success: " + fireflyToUpate.Id);
            }
            else
            {
                Debug.WriteLine("Error: Put Fail: " + fireflyToUpate.Id);
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
                Debug.WriteLine("Info: Put Success: " + nodeToUpate.Id);
            }
            else
            {
                Debug.WriteLine("Error: Put Fail: " + nodeToUpate.Id);
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
            RefreshNodesListView();

            if (((Pivot)sender).SelectedIndex == 0) // Fireflies page
            {
                RefreshNodesListView();
            }
            else if (((Pivot)sender).SelectedIndex == 1) // Nodes page
            {
                RefreshFirefliesListView();
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

        private void NavigationHelper_LoadState(object sender, LoadStateEventArgs e)
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
