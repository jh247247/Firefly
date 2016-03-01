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
using Windows.ApplicationModel.Resources;
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

        #region Contacts ListView

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

        private void RefreshContactsListView()
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
