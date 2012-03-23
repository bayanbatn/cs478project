package com.nvidia.fcamerapro;

import java.io.File;
import java.io.IOException;

import android.app.*;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.view.ActionMode;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.Gallery;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class EditorFragment extends Fragment implements FCamInterfaceEventListener, 
														OnItemLongClickListener, 
														OnItemSelectedListener, 
														OnItemClickListener, 
														OnSeekBarChangeListener, OnClickListener {
	/* Fragment Componentns */
	private View mContentView;
	private Gallery mStackGallery;
	private ImageStackManager mImageStackManager;
	private WebView mLargePreview;
	private View mGalleryPreview;	
	private SeekBar mFocusAdjustBar; //for adjusting focus distance
	private TextView mFocusTextView;
	final private Handler mHandler = new Handler();
	/*private Runnable mUpdateUITask = new Runnable() {
		public void run() {
			updateSeekBarValues(false); // Update sliders.
			mHandler.postDelayed(this, 1000 / Settings.UI_MAX_FPS);
		}
	};*/

	private ImageView mEditorView;
	private ProgressBar mEditorLoad;
	private double mMinFocus, mMaxFocus; 
	private Button mCaptureButton;
	private int mSelectedStack;
	
	private ActionMode.Callback mLargePreviewActionModeCallback = new ActionMode.Callback() {
		public boolean onCreateActionMode(ActionMode actionMode, Menu menu) {
			actionMode.setTitle(R.string.label_large_preview);
			mGalleryPreview.setVisibility(View.INVISIBLE);
			mEditorView.setVisibility(View.INVISIBLE);
			mLargePreview.setVisibility(View.VISIBLE);
			mContentView.setSystemUiVisibility(View.STATUS_BAR_HIDDEN);
			return true;
		}
		public boolean onPrepareActionMode(ActionMode actionMode, Menu menu) { return false; }
		public boolean onActionItemClicked(ActionMode actionMode, MenuItem menuItem) { return false; }
		public void onDestroyActionMode(ActionMode actionMode) {
			mLargePreview.loadUrl("about:blank");
			mLargePreview.setVisibility(View.INVISIBLE);
			mGalleryPreview.setVisibility(View.VISIBLE);
			mEditorView.setVisibility(View.VISIBLE);
			mContentView.setSystemUiVisibility(View.STATUS_BAR_VISIBLE);
		}
	};
	
	// Full initialization routine, called upon onCreate().
	private void initContentView() {
		if (mContentView != null) return; // return if already initialized.
		final FCameraPROActivity activity = ((FCameraPROActivity) getActivity());
		
		// Instantiate the stack manager.
		mImageStackManager = new ImageStackManager(activity.getStorageDirectory());

		// Load layout.
		mContentView = activity.getLayoutInflater().inflate(R.layout.editor, null);
		setHasOptionsMenu(true);

		TypedArray attr = activity.obtainStyledAttributes(R.styleable.Gallery);
		final int itemBackgroundStyleId = attr.getResourceId(R.styleable.Gallery_android_galleryItemBackground, 0);

		// Hooks for the individual UI components in the layout.
		mGalleryPreview = (View) mContentView.findViewById(R.id.editor_preview);
		mEditorView = (ImageView) mContentView.findViewById(R.id.editor_view);
		mEditorLoad = (ProgressBar) mContentView.findViewById(R.id.editor_busy);
		mLargePreview = (WebView) mContentView.findViewById(R.id.editor_large_preview);
		mLargePreview.setBackgroundColor(Color.BLACK);
		mLargePreview.getSettings().setBuiltInZoomControls(true);
		mLargePreview.getSettings().setUseWideViewPort(true);
		mLargePreview.getSettings().setLoadWithOverviewMode(true);
		
		// Hook for the stack gallery.
		mStackGallery = (Gallery) mContentView.findViewById(R.id.editor_stack_gallery);
		mStackGallery.setVerticalFadingEdgeEnabled(true);
		//mStackGallery.setVerticalScrollBarEnabled(true);
		//mStackGallery.setHorizontalFadingEdgeEnabled(true);
		mStackGallery.setAdapter(new BaseAdapter() {
			public int getCount() { return mImageStackManager.getStackCount(); }
			public Object getItem(int position) { return position; }
			public long getItemId(int position) { return position; }
			public View getView(int position, View convertView, ViewGroup parent) {
				View thumbnail;
				if (convertView != null) {
					thumbnail = convertView;
				} else {
					thumbnail = activity.getLayoutInflater().inflate(R.layout.thumbnail_editor, null);
					thumbnail.setBackgroundResource(itemBackgroundStyleId);
				}
				ImageView imageView = (ImageView) thumbnail.findViewById(R.id.thumbnail_image);
				ProgressBar busyBar = (ProgressBar) thumbnail.findViewById(R.id.thumbnail_busy);
				Bitmap bitmap = mImageStackManager.getStack(position).getImage(0).getThumbnail();
				if (bitmap != null) imageView.setImageBitmap(bitmap);

				busyBar.setVisibility(mImageStackManager.getStack(position).isLoadComplete() ? View.INVISIBLE : View.VISIBLE);
				return thumbnail;
			}
		});

		// Attach self as listeners. See the implementation of the listener
		// interfaces below this method.
		mStackGallery.setOnItemLongClickListener(this);
		mStackGallery.setOnItemSelectedListener(this);
		//mStackGallery.setOnItemClickListener(this);

		// ImageStackManager should listen to content changes.
		mImageStackManager.addContentChangeListener((BaseAdapter) mStackGallery.getAdapter());	
		
		//Focus adjust bar init
		mFocusAdjustBar = (SeekBar) mContentView.findViewById(R.id.sb_focus);
		mFocusAdjustBar.setMax(Settings.SEEK_BAR_PRECISION);
		mFocusAdjustBar.setOnSeekBarChangeListener(this);
		mFocusAdjustBar.setEnabled(true);
		//Set max and min range
		mMinFocus = 10.0;
		mMaxFocus = 0.0;
		//Label for focus distance
		mFocusTextView = (TextView) mContentView.findViewById(R.id.tv_focus);//TODO change this
		
		//Button push
		mCaptureButton = (Button) mContentView.findViewById(R.id.button_blur);
		mCaptureButton.setOnClickListener(this);
		
		mSelectedStack = 0;
	}
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		initContentView();
	}
	
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		try{
			FCamInterface.GetInstance().addEventListener(this);
		}catch(Exception e){}
		// Refresh image stacks upon creation, and move to the beginning of the stack.
		mImageStackManager.refreshImageStacks();
		if (mImageStackManager.getStackCount() != 0) {
			mHandler.post(new Runnable() {
				public void run() {
					mStackGallery.setSelection(0);
				}
			});
		}
		return mContentView;
	}
	
	@Override
	public void onCaptureStart() {
	}

	@Override
	public void onCaptureComplete() {
	}

	@Override
	public void onFileSystemChange() {
	}

	@Override
	public void onShotParamChange(int shotParamId) {
	}

	/* ====================================================================
	 * Implementation of OnItemSelectedListener interface.
	 * ==================================================================== */
	public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
		if (parent == mStackGallery) {
			Image image = mImageStackManager.getStack(position).getImage(0);
			mEditorView.setImageURI(Uri.fromFile(new File(image.getName())));
		} 
		mEditorLoad.setVisibility(mImageStackManager.getStack(position).isLoadComplete() ? View.INVISIBLE : View.VISIBLE);
		mSelectedStack = position;
		mImageStackManager.refreshImageStacks();
	}

	public void onNothingSelected(AdapterView<?> parent) {
	}

	/* ====================================================================
	 * Implementation of OnItemClickListener interface.
	 * ==================================================================== */
	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
		if (parent == mStackGallery) {
			Image image = mImageStackManager.getStack(position).getImage(0);
			mEditorView.setImageURI(Uri.fromFile(new File(image.getName())));
		} 
		mEditorLoad.setVisibility(mImageStackManager.getStack(position).isLoadComplete() ? View.INVISIBLE : View.VISIBLE);
		mSelectedStack = position;
		mImageStackManager.refreshImageStacks();
	}

	@Override
	/* ====================================================================
	 * Implementation of OnItemLongClickListener interface.
	 * ==================================================================== */
	// Long-clicking on an image gallery item should launch a preview for
	// the selected image.
	public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
		if (parent == mStackGallery) {
			Image image = mImageStackManager.getStack(position).getImage(0);
			if (image.getThumbnail() != null) {
				final FCameraPROActivity activity = ((FCameraPROActivity) getActivity());
				activity.startActionMode(mLargePreviewActionModeCallback);
				try {
					mLargePreview.loadUrl(new File(image.getName()).toURL().toString());
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			return true;
		}
		return false;
	}

	@Override
	public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
		FCamInterface iface = FCamInterface.GetInstance();
		if (seekBar == mFocusAdjustBar) {
			double focus = getFocusFromUI(progress);
			mFocusTextView.setText(Utils.FormatFocus(focus));
			if (seekBar.isEnabled()) {
				//iface.setPreviewParam(FCamInterface.PREVIEW_FOCUS, focus);
			}
		}
	}
	private double getFocusFromUI(int progress) {
		double nvalue = (double) progress / Settings.SEEK_BAR_PRECISION;
		return Math.pow(nvalue, Settings.SEEK_BAR_FOCUS_GAMMA) * (mMaxFocus - mMinFocus) + mMinFocus;
	}

	@Override
	public void onStartTrackingTouch(SeekBar seekBar) {}

	@Override
	public void onStopTrackingTouch(SeekBar seekBar) {}

	@Override
	public void onClick(View arg0) {
		FCamInterface iface = FCamInterface.GetInstance();
		
		int progress = mFocusAdjustBar.getProgress();
		float focus = (float) getFocusFromUI(progress);
		focus = 1 / focus;
		
		String depthFilePath = mImageStackManager.getStack(mSelectedStack).getImage(1).getName();
		String origFilePath = mImageStackManager.getStack(mSelectedStack).getImage(0).getName();
		
		iface.enqueueMessageForImageBlur(origFilePath, depthFilePath, focus);
		mImageStackManager.refreshImageStacks();
	}
	
}
