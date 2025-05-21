/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package is.xyz.filepicker;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.recyclerview.widget.RecyclerView;
import is.xyz.mpv.R;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.TypedValue;
import android.view.ViewGroup;

import java.util.List;

/**
 * A simple adapter which also inserts a header item ".." to handle going up to the parent folder.
 * @param <T> the type which is used, for example a normal java File object.
 */
class FileItemAdapter<T> extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

    protected final LogicHandler<T> mLogic;
    protected List<T> mList = null;
    protected SharedPreferences mSharedPrefs;
    protected int mHighlightColor;
    protected int mDefaultColor;

    public FileItemAdapter(@NonNull LogicHandler<T> logic, @NonNull Context context) {
        this.mLogic = logic;
        this.mSharedPrefs = PreferenceManager.getDefaultSharedPreferences(context);
        mHighlightColor = context.getResources().getColor(R.color.primary);
        TypedValue typedValue = new TypedValue();
        context.getTheme().resolveAttribute(android.R.attr.textColorPrimary, typedValue, true);
        mDefaultColor = typedValue.data;
    }

    public void setList(@Nullable List<T> list) {
        mList = list;
        notifyDataSetChanged();
    }

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return mLogic.onCreateViewHolder(parent, viewType);
    }

    @Override
    @SuppressWarnings("unchecked")
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder viewHolder, int headerPosition) {
        if (headerPosition == 0) {
            mLogic.onBindHeaderViewHolder((AbstractFilePickerFragment<T>.HeaderViewHolder) viewHolder);
        } else {
            int pos = headerPosition - 1;
            AbstractFilePickerFragment<T>.DirViewHolder dirViewHolder = (AbstractFilePickerFragment<T>.DirViewHolder) viewHolder;
            String selectedPath = mSharedPrefs.getString("selected_path", null);
            String path = mLogic.getFullPath(mList.get(pos));
            if (selectedPath != null && selectedPath.contains(path)) {
                dirViewHolder.text.setTextColor(mHighlightColor);
            } else {
                dirViewHolder.text.setTextColor(mDefaultColor);
            }
            mLogic.onBindViewHolder(dirViewHolder, pos, mList.get(pos));
        }
    }

    @Override
    public int getItemViewType(int headerPosition) {
        if (0 == headerPosition) {
            return LogicHandler.VIEWTYPE_HEADER;
        } else {
            int pos = headerPosition - 1;
            return mLogic.getItemViewType(pos, mList.get(pos));
        }
    }

    @Override
    public int getItemCount() {
        if (mList == null) {
            return 0;
        }

        // header + count
        return 1 + mList.size();
    }

    /**
     * Get the item at the designated position in the adapter.
     *
     * @param position of item in adapter
     * @return null if position is zero (that means it's the ".." header), the item otherwise.
     */
    protected @Nullable T getItem(int position) {
        if (position == 0) {
            return null;
        }
        return mList.get(position - 1);
    }
}
