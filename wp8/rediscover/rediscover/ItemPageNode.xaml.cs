using rediscover.Common;
using rediscover.DataModel;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
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

// The Pivot Application template is documented at http://go.microsoft.com/fwlink/?LinkID=391641

namespace rediscover
{
    /// <summary>
    /// A page that displays details for a single item within a group.
    /// </summary>
    public sealed partial class ItemPageNode : Page
    {
        private readonly NavigationHelper navigationHelper;
        private readonly ObservableDictionary defaultViewModel = new ObservableDictionary();
        SynchronizationContext _syncContext;

        private Node nodeClicked;
        private PivotPage pivotPageRef;
        private FireflyCollection nodeFireflies;

        public ItemPageNode()
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
        
        private void NavigationHelper_LoadState(object sender, LoadStateEventArgs e)
        {
            // Parse Navigation Args
            List<object> itemPageNavList = (List<object>) e.NavigationParameter;

            pivotPageRef = (PivotPage) itemPageNavList.ElementAt<object>(0);
            nodeClicked = (Node) itemPageNavList.ElementAt<object>(1);

            // Load details
            tblId.Text = nodeClicked.Id;

            if (nodeClicked.Location != "")
            {
                txtLocation.Text = nodeClicked.Location;
                txtLocation.FontStyle = Windows.UI.Text.FontStyle.Normal;
            }
            else
            {
                txtLocation.Text = "Not set!";
                txtLocation.FontStyle = Windows.UI.Text.FontStyle.Italic;
            }

            if (nodeClicked.LastUpdateTime != new DateTime(0))
            {
                tblLastUpdate.Text = nodeClicked.LastUpdateTime.ToString();
                tblLastUpdate.FontStyle = Windows.UI.Text.FontStyle.Normal;
            }
            else
            {
                tblLastUpdate.Text = "Unknown";
                tblLastUpdate.FontStyle = Windows.UI.Text.FontStyle.Italic;
            }

            if (nodeClicked.FirefliesList.Count != 0)
            {
                nodeFireflies = new FireflyCollection();

                foreach (JsonValue firefly in nodeClicked.FirefliesList)
                {
                    nodeFireflies.Add(pivotPageRef.fireflies.GetById(firefly.GetString()));
                }

                lstFireflies.ItemsSource = nodeFireflies;
            }
            else
            {
                lstFireflies.Visibility = Visibility.Collapsed;
            }
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

        private void txtLocation_KeyUp(object sender, KeyRoutedEventArgs e)
        {
            if (e.Key == Windows.System.VirtualKey.Enter)
            {
                // Hide keyboard
                InputPane.GetForCurrentView().TryHide();

                // Remove Textbox focus
                LoseFocus(sender);
            }
        }

        private void txtLocation_LostFocus(object sender, RoutedEventArgs e)
        {
            // Save
            nodeClicked.Location = txtLocation.Text;

            // Put
            pivotPageRef.putNodeLocation(nodeClicked);
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

        #region Firefly ListView

        private void lstFireflies_ItemClick(object sender, ItemClickEventArgs e)
        {
            // Get firefly
            var fireflyClicked = ((Firefly)e.ClickedItem);
            // Get actual firefly not just id
            //var fireflyClickedActual = pivotPageRef.fireflies.GetById(fireflyClicked.Id);

            // Setup Navigation args
            List<object> itemPageNavList = new List<object>();
            itemPageNavList.Add(pivotPageRef);
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
                lstFireflies.ItemsSource = nodeFireflies;
            }, null);
        }

        #endregion
    }
}