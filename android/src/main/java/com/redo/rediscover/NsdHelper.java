package com.redo.rediscover;

/**
 * This class should find our servers that advertise via
 * zeroconf/avahi/bonjour/whatever.
 * Refer to https://developer.android.com/reference/android/net/nsd/NsdManager.html for more information.
 * (C) Jack Hosemans 2015
 */

import java.util.ArrayList;
import java.lang.ref.WeakReference;

import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.content.Context;
import android.util.Log;

public class NsdHelper {
    static private String TAG = "NsdHelper";

    // TODO: confirm this...
    private static String SERVICE_NAME = "_rediscover._tcp";
    private NsdManager m_nsdManager; // very original name, I know.
    private NsdDiscovery m_discoveryListener = new NsdDiscovery();
    private NsdResolve m_resolveListener = new NsdResolve();
    // keep this as a weak reference just in case of context switches, so this class can be garbage collected.
    // have a method to set this...
    private WeakReference<NsdHelperListener> m_helperListener;

    public NsdHelper(Context ctx, NsdHelperListener l) {
	// note that we NEVER want to keep a strong reference to the context, otherwise we can get a memory leak and
	// that will just make jack sad.
	m_nsdManager = (NsdManager) ctx.getSystemService(Context.NSD_SERVICE);
	m_helperListener = new WeakReference<NsdHelperListener>(l);
    }

    public void startDiscovery() {
	m_nsdManager.discoverServices(SERVICE_NAME,
				      NsdManager.PROTOCOL_DNS_SD,
				      m_discoveryListener);
    }

    public void stopDiscovery() {
	try {
	    m_nsdManager.stopServiceDiscovery(m_discoveryListener);
	}
	finally {
	    // idk lol
	}
    }

    /**
     * This interface specifies the methods required for callbacks from this class.
     * onServiceResolved: called when a service matching the given SERVICE_NAME is resolved.
     * onServiceLost: called when the given service is lost. Note that all the details may not be in the given
     *     NsdServiceInfo, so rely on the name for comparison.
     */
    interface NsdHelperListener {
	public void onServiceResolved(NsdServiceInfo n);
	public void onServiceLost(NsdServiceInfo n);
    }

    // just in case of context switches, we can reset the listener.
    public void setListener(NsdHelperListener l) {
	m_helperListener = new WeakReference<NsdHelperListener>(l);
    }

    /**
     * This listens for network services that match our SERVICE_NAME.
     */
    class NsdDiscovery implements NsdManager.DiscoveryListener {
        @Override
        public void onDiscoveryStarted(String regType) {
	    Log.d(TAG,"Service discovery started!");
        }

        @Override
        public void onServiceFound(NsdServiceInfo service) {
	    Log.d(TAG,"Service found: " + service);
	    m_nsdManager.resolveService(service, m_resolveListener);
        }

        @Override
        public void onServiceLost(NsdServiceInfo service) {
	    Log.d(TAG,"Service lost " + service);

	    NsdHelperListener l = m_helperListener.get();
	    if(l != null) {
		l.onServiceLost(service);
	    }
        }

        @Override
        public void onDiscoveryStopped(String serviceType) {
	    Log.d(TAG,"Service discovery stopped!");
        }

        @Override
        public void onStartDiscoveryFailed(String serviceType,
                                           int errorCode) {
	    Log.d(TAG,"Start discovery failed. Error code" + errorCode);
        }

        @Override
        public void onStopDiscoveryFailed(String serviceType,
                                          int errorCode) {
	    Log.d(TAG,"Stop discovery failed. Error code" + errorCode);
        }
    }

    /**
     * Upon resolving the service, this listener should call the onServiceResolved callback in m_helperListener
     *
     */
    class NsdResolve implements NsdManager.ResolveListener {
	@Override
	public void onResolveFailed(NsdServiceInfo serviceInfo,
				    int errorCode) {
	    Log.d(TAG,"Resolve failed. Error code" + errorCode);
	}

	@Override
	public void onServiceResolved(NsdServiceInfo serviceInfo) {
	    Log.d(TAG,"Resolve succeeded: " + serviceInfo);
	    NsdHelperListener l = m_helperListener.get();
	    if(l != null) {
		l.onServiceResolved(serviceInfo);
	    }
	}
    }

}
