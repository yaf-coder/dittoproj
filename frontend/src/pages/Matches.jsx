import { useEffect, useState, useCallback } from 'react'
import { motion, AnimatePresence } from 'framer-motion'
import { api } from '../api/client'

function CheckIcon() {
  return (
    <svg width="32" height="32" viewBox="0 0 24 24" fill="none">
      <path d="M20 6L9 17l-5-5" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round"/>
    </svg>
  )
}

function XIcon() {
  return (
    <svg width="32" height="32" viewBox="0 0 24 24" fill="none">
      <path d="M18 6L6 18M6 6l12 12" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round"/>
    </svg>
  )
}

export default function Matches() {
  const [candidates, setCandidates] = useState([])
  const [index, setIndex]           = useState(0)
  const [loading, setLoading]       = useState(true)
  const [matched, setMatched]       = useState(false)
  const [direction, setDirection]   = useState(null)
  const [animating, setAnimating]   = useState(false)

  useEffect(() => {
    api.getCandidates()
      .then(setCandidates)
      .finally(() => setLoading(false))
  }, [])

  const current = candidates[index]

  const advance = useCallback((dir) => {
    if (animating || !current) return
    setDirection(dir)
    setAnimating(true)
  }, [animating, current])

  const onAnimationComplete = useCallback(() => {
    setAnimating(false)
    setMatched(false)
    setDirection(null)
    setIndex(i => i + 1)
  }, [])

  const handleLike = useCallback(async () => {
    if (!current) return
    advance('right')
    try {
      const res = await api.like(current.id)
      if (res.matched) setMatched(true)
    } catch {}
  }, [current, advance])

  const handlePass = useCallback(async () => {
    if (!current) return
    advance('left')
    try { await api.pass(current.id) } catch {}
  }, [current, advance])

  if (loading) {
    return (
      <div className="flex-1 flex items-center justify-center">
        <div className="w-8 h-8 border-[2.5px] border-apple-blue border-t-transparent rounded-full animate-spin" />
      </div>
    )
  }

  if (!current) {
    return (
      <div className="flex-1 flex flex-col items-center justify-center gap-3 px-8">
        <div className="w-20 h-20 rounded-full bg-apple-gray flex items-center justify-center">
          <svg width="36" height="36" viewBox="0 0 24 24" fill="none">
            <path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2" stroke="#6E6E73" strokeWidth="1.8" strokeLinecap="round"/>
            <circle cx="9" cy="7" r="4" stroke="#6E6E73" strokeWidth="1.8"/>
          </svg>
        </div>
        <p className="text-apple-sub font-medium text-[17px]">No more candidates</p>
        <p className="text-apple-sub/60 text-[14px] text-center">
          Check back later — more people may join.
        </p>
      </div>
    )
  }

  const exitX = direction === 'right' ? 400 : direction === 'left' ? -400 : 0

  return (
    <div className="flex-1 flex flex-col overflow-hidden select-none">
      <div className="px-6 pt-14 pb-2 safe-top shrink-0">
        <h1 className="text-[28px] font-bold tracking-tight text-apple-text">Discover</h1>
      </div>

      <div className="flex-1 flex items-center justify-center px-6 relative overflow-hidden">
        <AnimatePresence onExitComplete={onAnimationComplete}>
          {!animating && (
            <motion.div
              key={current.id}
              initial={{ opacity: 0, scale: 0.95 }}
              animate={{ opacity: 1, scale: 1 }}
              exit={{ x: exitX, opacity: 0, rotate: exitX > 0 ? 12 : -12 }}
              transition={{ duration: 0.35, ease: [0.25, 0.46, 0.45, 0.94] }}
              className="w-full max-w-sm rounded-3xl overflow-hidden shadow-apple bg-white"
            >
              <div className="relative w-full aspect-[3/4] bg-apple-gray">
                {current.picture ? (
                  <img
                    src={current.picture}
                    alt={current.name}
                    className="w-full h-full object-cover"
                  />
                ) : (
                  <div className="w-full h-full flex items-center justify-center">
                    <span className="text-6xl font-bold text-apple-sub/30">
                      {current.name?.[0]}
                    </span>
                  </div>
                )}
                <div className="absolute inset-0 bg-gradient-to-t from-black/60 via-transparent to-transparent" />
                <div className="absolute bottom-0 left-0 right-0 p-5">
                  <p className="text-white font-bold text-[24px] leading-tight">
                    {current.name}{current.age ? `, ${current.age}` : ''}
                  </p>
                  {current.compatibility != null && (
                    <p className="text-white/80 text-[13px] mt-1">
                      {Math.round(current.compatibility * 100)}% match
                    </p>
                  )}
                </div>
              </div>
            </motion.div>
          )}
        </AnimatePresence>

        <AnimatePresence>
          {matched && (
            <motion.div
              key="matched"
              initial={{ opacity: 0, scale: 0.8 }}
              animate={{ opacity: 1, scale: 1 }}
              exit={{ opacity: 0, scale: 1.1 }}
              className="absolute inset-0 flex items-center justify-center bg-black/40 rounded-3xl z-10"
            >
              <div className="bg-white rounded-2xl px-8 py-6 flex flex-col items-center gap-2 shadow-apple">
                <span className="text-3xl">🎉</span>
                <p className="font-bold text-[20px] text-apple-text">It's a Match!</p>
              </div>
            </motion.div>
          )}
        </AnimatePresence>
      </div>

      <div className="shrink-0 flex items-center justify-center gap-10 pb-10 pt-6">
        <motion.button
          whileTap={{ scale: 0.9 }}
          onClick={handlePass}
          disabled={animating}
          className="w-16 h-16 rounded-full bg-white shadow-apple flex items-center justify-center text-red-500 disabled:opacity-40"
        >
          <XIcon />
        </motion.button>

        <motion.button
          whileTap={{ scale: 0.9 }}
          onClick={handleLike}
          disabled={animating}
          className="w-16 h-16 rounded-full bg-white shadow-apple flex items-center justify-center text-green-500 disabled:opacity-40"
        >
          <CheckIcon />
        </motion.button>
      </div>
    </div>
  )
}
