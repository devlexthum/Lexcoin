// Copyright (c) 2020-2022 The Lex Core developers
# Distributed under the MIT software license, see the accompanying

#ifndef LEX_CRYPTO_BLS_WORKER_H
#define LEX_CRYPTO_BLS_WORKER_H

#include "bls.h"

#include "ctpl.h"

#include <future>
#include <mutex>

#include <boost/lockfree/queue.hpp>

class CBLSWorker
{
public:
    typedef std::function<void(const CBLSSignature&)> SignDoneCallback;
    typedef std::function<void(bool)> SigVerifyDoneCallback;
    typedef std::function<bool()> CancelCond;

private:
    ctpl::thread_pool workerPool;

    static const int SIG_VERIFY_BATCH_SIZE = 8;
    struct SigVerifyJob {
        SigVerifyDoneCallback doneCallback;
        CancelCond cancelCond;
        CBLSSignature sig;
        CBLSPublicKey pubKey;
        uint256 msgHash;
        SigVerifyJob(SigVerifyDoneCallback&& _doneCallback, CancelCond&& _cancelCond, const CBLSSignature& _sig, const CBLSPublicKey& _pubKey, const uint256& _msgHash) :
            doneCallback(_doneCallback),
            cancelCond(_cancelCond),
            sig(_sig),
            pubKey(_pubKey),
            msgHash(_msgHash)
        {
        }
    };

    std::mutex sigVerifyMutex;
    int sigVerifyBatchesInProgress{0};
    std::vector<SigVerifyJob> sigVerifyQueue;

public:
    CBLSWorker();
    ~CBLSWorker();

    void Start();
    void Stop();

    bool GenerateContributions(int threshold, const BLSIdVector& ids, BLSVerificationVectorPtr& vvecRet, BLSSecretKeyVector& skShares);

    // The following functions are all used to aggregate verification (public key) vectors
    // Inputs are in the following form:
    //   [
    void AsyncBuildQuorumVerificationVector(const std::vector<BLSVerificationVectorPtr>& vvecs,
                                            size_t start, size_t count, bool parallel,
                                            std::function<void(const BLSVerificationVectorPtr&)> doneCallback);
    std::future<BLSVerificationVectorPtr> AsyncBuildQuorumVerificationVector(const std::vector<BLSVerificationVectorPtr>& vvecs,
                                                                             size_t start, size_t count, bool parallel);
    BLSVerificationVectorPtr BuildQuorumVerificationVector(const std::vector<BLSVerificationVectorPtr>& vvecs,
                                                           size_t start = 0, size_t count = 0, bool parallel = true);

    // Aggregation is paralellized by splitting up the input vector into multiple batches and then aggregating the individual batch results
    void AsyncAggregateSecretKeys(const BLSSecretKeyVector& secKeys,
                                  size_t start, size_t count, bool parallel,
                                  std::function<void(const CBLSSecretKey&)> doneCallback);
    std::future<CBLSSecretKey> AsyncAggregateSecretKeys(const BLSSecretKeyVector& secKeys,
                                                        size_t start, size_t count, bool parallel);
    CBLSSecretKey AggregateSecretKeys(const BLSSecretKeyVector& secKeys, size_t start = 0, size_t count = 0, bool parallel = true);

    void AsyncAggregatePublicKeys(const BLSPublicKeyVector& pubKeys,
                                  size_t start, size_t count, bool parallel,
                                  std::function<void(const CBLSPublicKey&)> doneCallback);
    std::future<CBLSPublicKey> AsyncAggregatePublicKeys(const BLSPublicKeyVector& pubKeys,
                                                        size_t start, size_t count, bool parallel);
    CBLSPublicKey AggregatePublicKeys(const BLSPublicKeyVector& pubKeys, size_t start = 0, size_t count = 0, bool parallel = true);

    void AsyncAggregateSigs(const BLSSignatureVector& sigs,
                            size_t start, size_t count, bool parallel,
                            std::function<void(const CBLSSignature&)> doneCallback);
    std::future<CBLSSignature> AsyncAggregateSigs(const BLSSignatureVector& sigs,
                                                        size_t start, size_t count, bool parallel);
    CBLSSignature AggregateSigs(const BLSSignatureVector& sigs, size_t start = 0, size_t count = 0, bool parallel = true);


    // Calculate public key share from public key vector and id. Not parallelized
    CBLSPublicKey BuildPubKeyShare(const BLSVerificationVectorPtr& vvec, const CBLSId& id);

    // The following functions verify multiple verification vectors and contributions for the same id
    // This is parallelized by performing batched verification. The verification vectors and the contributions of
    void AsyncVerifyContributionShares(const CBLSId& forId, const std::vector<BLSVerificationVectorPtr>& vvecs, const BLSSecretKeyVector& skShares,
                                       bool parallel, bool aggregated, std::function<void(const std::vector<bool>&)> doneCallback);
    std::future<std::vector<bool> > AsyncVerifyContributionShares(const CBLSId& forId, const std::vector<BLSVerificationVectorPtr>& vvecs, const BLSSecretKeyVector& skShares,
                                                                  bool parallel, bool aggregated);
    std::vector<bool> VerifyContributionShares(const CBLSId& forId, const std::vector<BLSVerificationVectorPtr>& vvecs, const BLSSecretKeyVector& skShares,
                                               bool parallel = true, bool aggregated = true);

    std::future<bool> AsyncVerifyContributionShare(const CBLSId& forId, const BLSVerificationVectorPtr& vvec, const CBLSSecretKey& skContribution);

    // Non paralellized verification of a single contribution
    bool VerifyContributionShare(const CBLSId& forId, const BLSVerificationVectorPtr& vvec, const CBLSSecretKey& skContribution);

    // Simple verification of vectors. Checks x.IsValid() for every entry and checks for duplicate entries
    bool VerifyVerificationVector(const BLSVerificationVector& vvec, size_t start = 0, size_t count = 0);
    bool VerifyVerificationVectors(const std::vector<BLSVerificationVectorPtr>& vvecs, size_t start = 0, size_t count = 0);
    bool VerifySecretKeyVector(const BLSSecretKeyVector& secKeys, size_t start = 0, size_t count = 0);
    bool VerifySignatureVector(const BLSSignatureVector& sigs, size_t start = 0, size_t count = 0);

    // Internally batched signature signing and verification
    void AsyncSign(const CBLSSecretKey& secKey, const uint256& msgHash, SignDoneCallback doneCallback);
    std::future<CBLSSignature> AsyncSign(const CBLSSecretKey& secKey, const uint256& msgHash);
    void AsyncVerifySig(const CBLSSignature& sig, const CBLSPublicKey& pubKey, const uint256& msgHash, SigVerifyDoneCallback doneCallback, CancelCond cancelCond = [] { return false; });
    std::future<bool> AsyncVerifySig(const CBLSSignature& sig, const CBLSPublicKey& pubKey, const uint256& msgHash, CancelCond cancelCond = [] { return false; });
    bool IsAsyncVerifyInProgress();

private:
    void PushSigVerifyBatch();
};

// Builds and caches different things from CBLSWorker
// Cache keys are provided externally as computing hashes on BLS vectors is too expensive
class CBLSWorkerCache
{
private:
    CBLSWorker& worker;

    std::mutex cacheCs;
    std::map<uint256, std::shared_future<BLSVerificationVectorPtr> > vvecCache;
    std::map<uint256, std::shared_future<CBLSSecretKey> > secretKeyShareCache;
    std::map<uint256, std::shared_future<CBLSPublicKey> > publicKeyShareCache;

public:
    CBLSWorkerCache(CBLSWorker& _worker) :
        worker(_worker) {}

    BLSVerificationVectorPtr BuildQuorumVerificationVector(const uint256& cacheKey, const std::vector<BLSVerificationVectorPtr>& vvecs)
    {
        return GetOrBuild(cacheKey, vvecCache, [&]() {
            return worker.BuildQuorumVerificationVector(vvecs);
        });
    }
    CBLSSecretKey AggregateSecretKeys(const uint256& cacheKey, const BLSSecretKeyVector& skShares)
    {
        return GetOrBuild(cacheKey, secretKeyShareCache, [&]() {
            return worker.AggregateSecretKeys(skShares);
        });
    }
    CBLSPublicKey BuildPubKeyShare(const uint256& cacheKey, const BLSVerificationVectorPtr& vvec, const CBLSId& id)
    {
        return GetOrBuild(cacheKey, publicKeyShareCache, [&]() {
            return worker.BuildPubKeyShare(vvec, id);
        });
    }

private:
    template <typename T, typename Builder>
    T GetOrBuild(const uint256& cacheKey, std::map<uint256, std::shared_future<T> >& cache, Builder&& builder)
    {
        cacheCs.lock();
        auto it = cache.find(cacheKey);
        if (it != cache.end()) {
            auto f = it->second;
            cacheCs.unlock();
            return f.get();
        }

        std::promise<T> p;
        cache.emplace(cacheKey, p.get_future());
        cacheCs.unlock();

        T v = builder();
        p.set_value(v);
        return v;
    }
};

#endif //LEX_CRYPTO_BLS_WORKER_H
