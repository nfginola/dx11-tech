#include "pch.h"
#include "Graphics/GfxHelperTypes.h"
#include "Timer.h"

void GPUProfiler::begin(const std::string& name, bool annotate, bool get_pipeline_stats)
{
    ProfileData& profile = m_profiles[name];
    assert(profile.query_started == false);
    //assert(profile.query_finished == false);

    if (!profile.disjoint[m_curr_frame])
    {
        auto& dev = m_dev->get_device();
        D3D11_QUERY_DESC desc;
        desc.MiscFlags = 0;

        // Create two timestamp queries and a disjoint query
        desc.Query = D3D11_QUERY_TIMESTAMP;
        HRCHECK(dev->CreateQuery(&desc, &profile.timestamp_start[m_curr_frame]));
        HRCHECK(dev->CreateQuery(&desc, &profile.timestamp_end[m_curr_frame]));

        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        HRCHECK(dev->CreateQuery(&desc, &profile.disjoint[m_curr_frame]));

        if (get_pipeline_stats)
        {
            desc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
            HRCHECK(dev->CreateQuery(&desc, &profile.pipeline_statistics[m_curr_frame]));
        }
    }

    auto& ctx = m_dev->get_context();

    // Start a disjoint query first
    ctx->Begin(profile.disjoint[m_curr_frame].Get());

    // Insert the start timestamp    
    ctx->End(profile.timestamp_start[m_curr_frame].Get());

    // Start pipeline statistics
    if (get_pipeline_stats)
        ctx->Begin(profile.pipeline_statistics[m_curr_frame].Get());

    profile.query_started = true;

    profile.annotate = annotate;
    if (annotate)
        m_dev->get_annotation()->BeginEvent(utils::to_wstr(name.c_str()).c_str());
}

void GPUProfiler::end(const std::string& name)
{
    ProfileData& profile = m_profiles[name];
    assert(profile.query_started == true);
    //assert(profile.query_finished == false);

    if (profile.annotate)
        m_dev->get_annotation()->EndEvent();

    auto& ctx = m_dev->get_context();

    // Insert the end timestamp    
    ctx->End(profile.timestamp_end[m_curr_frame].Get());

    // End the disjoint query
    ctx->End(profile.disjoint[m_curr_frame].Get());

    // End pipeline statistics
    if (profile.pipeline_statistics[m_curr_frame])
        ctx->End(profile.pipeline_statistics[m_curr_frame].Get());

    profile.query_started = false;
    profile.query_finished = true;
    m_frame_finished = true;
}

const GPUProfiler::FrameData& GPUProfiler::get_frame_statistics()
{
    assert(m_frame_finished == true);
    return m_frame_datas[m_curr_frame]; // Get the oldest frame data (this func is called after frame_end())
}

void GPUProfiler::frame_start()
{
    begin("*** Full Frame ***", false);
}

void GPUProfiler::frame_end()
{
    end("*** Full Frame ***");

    auto& ctx = m_dev->get_context();

    // Go to "oldest frame" in list
    ++m_curr_frame;
    m_curr_frame = m_curr_frame % gfxconstants::QUERY_LATENCY;

    float waiting_time = 0;
    // go over each profile and extract
    FrameData frame_data{};
    for (const auto& it : m_profiles)
    {
        const auto& name = it.first;
        const auto& profile = it.second;

        Timer timer;
        // Get the query data
        UINT64 start_time = 0;
        if (profile.timestamp_start[m_curr_frame])
            while (ctx->GetData(profile.timestamp_start[m_curr_frame].Get(), &start_time, sizeof(start_time), 0) != S_OK);

        UINT64 end_time = 0;
        if (profile.timestamp_end[m_curr_frame])
            while (ctx->GetData(profile.timestamp_end[m_curr_frame].Get(), &end_time, sizeof(end_time), 0) != S_OK);

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData{};
        if (profile.disjoint[m_curr_frame])
            while (ctx->GetData(profile.disjoint[m_curr_frame].Get(), &disjointData, sizeof(disjointData), 0) != S_OK);
        waiting_time += timer.elapsed();

        D3D11_QUERY_DATA_PIPELINE_STATISTICS pipeline_stats{};
        if (profile.pipeline_statistics[m_curr_frame])
            while (ctx->GetData(profile.pipeline_statistics[m_curr_frame].Get(), &pipeline_stats, sizeof(pipeline_stats), 0) != S_OK);
        waiting_time += timer.elapsed();

        // Convert delta to ms
        float time = 0.0f;
        if (profile.disjoint[m_curr_frame] && disjointData.Disjoint == false)
        {
            UINT64 delta = end_time - start_time;
            float frequency = static_cast<float>(disjointData.Frequency);
            time = (delta / frequency) * 1000.0f;
        }

        if (pipeline_stats.PSInvocations != 0)
            frame_data.profiles.insert({ name, { pipeline_stats, time } });
        else
            frame_data.profiles.insert({ name, { {}, time } });
    }

    frame_data.query_waiting_time = waiting_time;
    m_frame_datas[m_curr_frame] = frame_data;
}

void GPUAnnotator::begin_event(const std::string& name)
{
    m_annotation->BeginEvent(utils::to_wstr(name).c_str());
}

void GPUAnnotator::end_event()
{
    m_annotation->EndEvent();
}

void GPUAnnotator::set_marker(const std::string& name)
{
    m_annotation->SetMarker(utils::to_wstr(name).c_str());
}
