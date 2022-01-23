#include "pch.h"
#include "Graphics/GfxTypes.h"

void GPUProfiler::begin_profile(const std::string& name)
{
	ProfileData& profile = m_profiles[name];
	assert(profile.query_started == false);
	//assert(profile.query_finished == false);

    if (!profile.disjoint)
    {
        auto& dev = m_dev->get_device();
        D3D11_QUERY_DESC desc;
        desc.MiscFlags = 0;

        // Create two timestamp queries and a disjoint query
        desc.Query = D3D11_QUERY_TIMESTAMP;
        HRCHECK(dev->CreateQuery(&desc, &profile.timestamp_start));
        HRCHECK(dev->CreateQuery(&desc, &profile.timestamp_end));

        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        HRCHECK(dev->CreateQuery(&desc, &profile.disjoint));
    }

    auto& ctx = m_dev->get_context();

    // Start a disjoint query first
    ctx->Begin(profile.disjoint.Get());

    // Insert the start timestamp    
    ctx->End(profile.timestamp_start.Get());

    profile.query_started = true;

}

void GPUProfiler::end_profile(const std::string& name)
{
    ProfileData& profile = m_profiles[name];
    assert(profile.query_started == true);
    //assert(profile.query_finished == false);

    auto& ctx = m_dev->get_context();

    // Insert the end timestamp    
    ctx->End(profile.timestamp_end.Get());

    // End the disjoint query
    ctx->End(profile.disjoint.Get());

    profile.query_started = false;
    profile.query_finished = true;
}

const GPUProfiler::FrameData& GPUProfiler::get_frame_statistics()
{
	assert(m_frame_finished == true);
	return m_frame_data;
}

void GPUProfiler::frame_start()
{
}

void GPUProfiler::frame_end()
{
    auto& ctx = m_dev->get_context();

    // go over each profile and extract
    for (const auto& it : m_profiles)
    {
        const auto& name = it.first;
        const auto& profile = it.second;

        // Get the query data
        UINT64 start_time = 0;
        while (ctx->GetData(profile.timestamp_start.Get(), &start_time, sizeof(start_time), 0) != S_OK);

        UINT64 end_time = 0;
        while (ctx->GetData(profile.timestamp_end.Get(), &end_time, sizeof(end_time), 0) != S_OK);

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
        while (ctx->GetData(profile.disjoint.Get(), &disjointData, sizeof(disjointData), 0) != S_OK);

        // Convert delta to ms
        float time = 0.0f;
        if (disjointData.Disjoint == false)
        {
            UINT64 delta = end_time - start_time;
            float frequency = static_cast<float>(disjointData.Frequency);
            time = (delta / frequency) * 1000.0f;
        }

        std::cout << std::fixed;
        std::cout << std::setprecision(3);
        std::cout << time << " ms : " << name << "\n";
    }
    std::cout << "\n";
}
