import { useCallback, useEffect, useState } from 'react'
import { motion, AnimatePresence } from 'framer-motion'
import { api } from '../api/client'
import SwipeCard from '../components/SwipeCard'
import Button    from '../components/Button'

export default function Discover() {
  const [stack, setStack]     = useState([])
  const [loading, setLoading] = useState(true)
  const [matched, setMatched] = useState(false)  // flash on mutual match

  const load = useCallback(async () => {
    setLoading(true)
    try {
      const profiles = await api.discover()
      setStack(profiles)
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => { load() }, [load])

  async function handleLike() {
    const top = stack[stack.length - 1]
    if (!top) return
    setStack(s => s.slice(0, -1))
    try {
      const res = await api.like(top.id)
      if (res.matched) {
        setMatched(true)
        setTimeout(() => setMatched(false), 2000)
      }
    } catch {}
  }

  function handlePass() {
    setStack(s => s.slice(0, -1))
  }

  if (loading) {
    return (
      <div className="flex-1 flex items-center justify-center">
        <div className="w-10 h-10 border-[3px] border-apple-blue border-t-transparent rounded-full animate-spin" />
      </div>
    )
  }

  return (
    <div className="flex-1 flex flex-col overflow-hidden">
      {/* Header */}
      <div className="px-6 pt-14 pb-4 safe-top">
        <h1 className="text-[28px] font-bold tracking-tight2 text-apple-text">Discover</h1>
      </div>

      {/* Card stack */}
      <div className="flex-1 relative px-4 pb-4">
        {stack.length === 0 ? (
          <div className="absolute inset-0 flex flex-col items-center justify-center gap-4">
            <div className="w-20 h-20 rounded-full bg-apple-gray flex items-center justify-center">
              <svg width="36" height="36" viewBox="0 0 24 24" fill="none">
                <path d="M12 21C12 21 3 15 3 8.5A5 5 0 0 1 12 6a5 5 0 0 1 9 2.5C21 15 12 21 12 21z"
                  stroke="#6E6E73" strokeWidth="1.8" strokeLinejoin="round"/>
              </svg>
            </div>
            <p className="text-apple-sub font-medium text-[17px]">You've seen everyone</p>
            <Button variant="secondary" onClick={load}>Refresh</Button>
          </div>
        ) : (
          <>
            {/* Cards behind — decorative */}
            {stack.slice(0, -1).slice(-2).map((profile, i, arr) => {
              const depth   = arr.length - i
              const scale   = 1 - depth * 0.04
              const offsetY = depth * 12
              return (
                <div
                  key={profile.id}
                  className="absolute inset-x-4"
                  style={{
                    top: 0, bottom: 0,
                    transform: `translateY(${offsetY}px) scale(${scale})`,
                    transformOrigin: 'bottom center',
                    zIndex: i,
                  }}
                >
                  <div className="w-full h-full rounded-4xl bg-apple-gray shadow-apple-sm overflow-hidden">
                    {profile.picture && (
                      <img src={profile.picture} alt="" className="w-full h-full object-cover opacity-60" />
                    )}
                  </div>
                </div>
              )
            })}

            {/* Top card — draggable */}
            <AnimatePresence>
              <SwipeCard
                key={stack[stack.length - 1].id}
                profile={stack[stack.length - 1]}
                onLike={handleLike}
                onPass={handlePass}
                style={{ zIndex: 10 }}
              />
            </AnimatePresence>
          </>
        )}
      </div>

      {/* Action buttons */}
      {stack.length > 0 && (
        <div className="flex justify-center items-center gap-6 pb-6">
          <button
            onClick={handlePass}
            className="w-16 h-16 rounded-full bg-white shadow-apple flex items-center justify-center
                       active:scale-90 transition-transform"
          >
            <svg width="28" height="28" viewBox="0 0 24 24" fill="none">
              <path d="M18 6L6 18M6 6l12 12" stroke="#FF3B30" strokeWidth="2.2" strokeLinecap="round"/>
            </svg>
          </button>
          <button
            onClick={handleLike}
            className="w-20 h-20 rounded-full bg-apple-blue shadow-apple-lg flex items-center justify-center
                       active:scale-90 transition-transform"
          >
            <svg width="34" height="34" viewBox="0 0 24 24" fill="white">
              <path d="M12 21C12 21 3 15 3 8.5A5 5 0 0 1 12 6a5 5 0 0 1 9 2.5C21 15 12 21 12 21z"/>
            </svg>
          </button>
        </div>
      )}

      {/* Match flash */}
      <AnimatePresence>
        {matched && (
          <motion.div
            initial={{ opacity: 0, scale: 0.85 }}
            animate={{ opacity: 1, scale: 1 }}
            exit={{ opacity: 0, scale: 0.85 }}
            className="absolute inset-0 flex items-center justify-center bg-black/40 z-50"
          >
            <div className="bg-white rounded-4xl px-10 py-10 mx-8 text-center shadow-apple-lg">
              <p className="text-5xl mb-3">💫</p>
              <h2 className="text-[28px] font-bold tracking-tight2 mb-1">It's a Match!</h2>
              <p className="text-apple-sub text-[15px]">You both liked each other.</p>
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </div>
  )
}
