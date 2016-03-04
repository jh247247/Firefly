using rediscover.DataModel;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace rediscover
{
    public sealed partial class ItemViewerNode : UserControl
    {
        public ItemViewerNode()
        {
            this.InitializeComponent();
        }

        public void ShowPlaceholder(Node item)
        {
            _node = item;
            txtNodeId.Opacity = 0;
            txtNodeLocation.Opacity = 0;
        }

        public void ShowId()
        {
            txtNodeId.Text = _node.Id;
            txtNodeId.Opacity = 1;
        }

        public void ShowLocation()
        {
            txtNodeLocation.Text = _node.Location;
            txtNodeLocation.Opacity = 1;
        }

        public void ClearData()
        {
            _node = null;
            txtNodeId.ClearValue(TextBlock.TextProperty);
            txtNodeLocation.ClearValue(TextBlock.TextProperty);
        }

        private Node _node;
    }
}
