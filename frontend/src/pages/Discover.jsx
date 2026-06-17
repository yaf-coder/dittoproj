import { useCallback, useEffect, useState } from 'react'
import { motion, AnimatePresence } from 'framer-motion'
import { api } from '../api/client'
import SwipeCard from '../components/SwipeCard'
import Button    from '../components/Button'

// Fallback: New Haven, CT (the restaurants in Matches detail are from here).
const FALLBACK_LOCATION = { lat: 41.3083, lng: -72.9279 }

export default function Discover() {
  const [stack, setStack]     = useState([])
  const [loading, setLoading] = useState(true)
  const [matched, setMatched] = useState(false)  // flash on mutual match
  const [location, setLocation] = useState(null)
  const [searchQuery, setSearchQuery] = useState('')
  const [submittedQuery, setSubmittedQuery] = useState('')

  const load = useCallback(async () => {
    setLoading(true)
    try {
      const profiles = await api.getCandidates()
      setStack([...profiles].reverse())
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => { load() }, [load])

  useEffect(() => {
    if (!navigator.geolocation) {
      setLocation(FALLBACK_LOCATION)
      return
    }
    navigator.geolocation.getCurrentPosition(
      (pos) => setLocation({ lat: pos.coords.latitude, lng: pos.coords.longitude }),
      ()    => setLocation(FALLBACK_LOCATION),
      { timeout: 5000 }
    )
  }, [])

  const mapsSrc = location
    ? submittedQuery
      ? `https://maps.google.com/maps?q=${encodeURIComponent(submittedQuery)}&ll=${location.lat},${location.lng}&z=14&output=embed`
      : `https://maps.google.com/maps?q=${location.lat},${location.lng}&z=14&output=embed`
    : null

  function handleSearch(e) {
    e.preventDefault()
    setSubmittedQuery(searchQuery.trim())
  }

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

  async function handlePass() {
    const top = stack[stack.length - 1]
    if (!top) return
    setStack(s => s.slice(0, -1))
    try { await api.pass(top.id) } catch {}
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
      <div className="px-6 pt-5 pb-4">
        <h1 className="text-lg font-cursive text-apple-text">Discover</h1>
      </div>

      {/* Two-column layout: Maps left | Card stack right */}
      <div className="flex-1 flex gap-4 px-4 pb-8 overflow-hidden">
        {/* Left: Google Maps */}
        <div className="hidden md:flex flex-1 flex-col gap-2 min-w-0">
          <form onSubmit={handleSearch} className="relative px-1">
            <svg
              width="16" height="16" viewBox="0 0 24 24" fill="none"
              className="absolute left-5 top-1/2 -translate-y-1/2 pointer-events-none"
            >
              <circle cx="11" cy="11" r="7" stroke="#8D8FA8" strokeWidth="2"/>
              <path d="M20 20l-3.5-3.5" stroke="#8D8FA8" strokeWidth="2" strokeLinecap="round"/>
            </svg>
            <input
              type="text"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              placeholder="Search restaurants, cafes, bars…"
              className="w-full pl-10 pr-4 py-2.5 rounded-full bg-white shadow-apple-sm text-[14px] text-apple-text placeholder:text-apple-sub/60 focus:outline-none focus:ring-2 focus:ring-apple-blue/40"
            />
            {submittedQuery && (
              <button
                type="button"
                onClick={() => { setSearchQuery(''); setSubmittedQuery('') }}
                className="absolute right-3 top-1/2 -translate-y-1/2 text-[11px] font-cursive text-apple-blue px-2 py-1"
              >
                Clear
              </button>
            )}
          </form>
          <div className="flex-1 rounded-3xl overflow-hidden bg-apple-gray shadow-apple-sm">
            {mapsSrc ? (
              <iframe
                title="Google Maps"
                src={mapsSrc}
                className="w-full h-full border-0"
                loading="lazy"
                referrerPolicy="no-referrer-when-downgrade"
              />
            ) : (
              <div className="w-full h-full flex items-center justify-center text-apple-sub text-sm">
                Locating…
              </div>
            )}
          </div>
        </div>

        {/* Right: Swipe card stack */}
        <div className="flex items-center justify-center shrink-0">
          <div className="relative h-full max-h-[80vh]" style={{ aspectRatio: '3/4' }}>
          {stack.length === 0 ? (
            <div className="absolute inset-0 flex flex-col items-center justify-center gap-4">
              <div className="w-20 h-20 rounded-full bg-apple-gray flex items-center justify-center">
                <svg width="36" height="36" viewBox="0 0 24 24" fill="none">
                  <path d="M12 21C12 21 3 15 3 8.5A5 5 0 0 1 12 6a5 5 0 0 1 9 2.5C21 15 12 21 12 21z"
                    stroke="#8D8FA8" strokeWidth="1.8" strokeLinejoin="round"/>
                </svg>
              </div>
              <p className="text-apple-sub font-medium text-[17px]">You've seen everyone</p>
              <Button variant="secondary" onClick={load}>Refresh</Button>
            </div>
          ) : (
            <>
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
        </div>
      </div>


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
              <h2 className="text-lg font-bold tracking-tight2 mb-1">It's a Match!</h2>
              <p className="text-apple-sub text-[15px]">You both liked each other.</p>
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </div>
  )
}
